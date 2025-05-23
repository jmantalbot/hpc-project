#include <iostream>
#include <string>
#include <vector>
#include "rapidcsv.h"
#include "point.hpp"
#include <map>
#ifdef MPI_TARGET
	#include "distributed_cluster.hpp"
  #include <boost/mpi/environment.hpp>
  #include <boost/mpi/communicator.hpp>
  #include <boost/serialization/vector.hpp>
  #include <boost/mpi/collectives.hpp>
#endif
#ifdef OMP_TARGET
	#include <omp.h>
	#include "shared_cluster.hpp"
#endif
#ifdef SERIAL_TARGET
	#include "shared_cluster.hpp"
#endif
#ifdef CUDA_TARGET
	#include "cuda_cluster.hpp"
#endif

std::vector<Point> readInputData(std::string filepath) {
	rapidcsv::Document doc(filepath);

	// for now, ignoring id,name,album,album_id,artists,artist_ids,track_number,disc_number,explicit,duration_ms,year,release_date
	const std::vector<std::string> FEATURE_KEYS = {
		"explicit",
		"danceability",
		"energy",
		"key",
		"loudness",
		"mode",
		"speechiness",
		"acousticness",
		"instrumentalness",
		"liveness",
		"valence",
		"tempo",
		"time_signature",
	};


	size_t row_count = doc.GetRowCount();
	std::vector<Point> input_data(row_count);
	#ifdef OMP_TARGET
	#pragma omp parallel for
	#endif
	for (int row_idx = 0; row_idx < static_cast<int>(row_count); row_idx++) {
		//for each row
		std::vector<std::string> row = doc.GetRow<std::string>(row_idx);
		std::vector<float> coordinates;
		coordinates.reserve(FEATURE_KEYS.size());
		#ifdef OMP_TARGET
		#pragma omp parallel for
		#endif
		for (size_t feature_idx = 0; feature_idx < FEATURE_KEYS.size(); feature_idx++) {
			std::string feature = doc.GetCell<std::string>(FEATURE_KEYS[feature_idx], row_idx);
			//try to convert the feature to a float. Expecting strings to look like integers, floats, or boolean "True"/"False"
			if (feature == "True") {
				coordinates.push_back(1.0);
			}
			else if (feature == "False") {
				coordinates.push_back(0.0);
			}
			else {
				coordinates.push_back(std::stof(feature));
			}
		}
		input_data[row_idx] = Point(coordinates);
	}
	return input_data;
}

void writeClusterData(std::string inputFilepath, std::string outputFilepath, std::vector<Point>* points) {
	std::vector<int> clusters(points->size());
	#ifdef OMP_TARGET
	#pragma omp parallel for
	#endif
	for (int i = 0; i < static_cast<int>(points->size()); i++) {
		clusters[i] = points->at(i).cluster;
	}
	rapidcsv::Document doc(inputFilepath);
	doc.InsertColumn<int>(doc.GetColumnCount(), clusters, "cluster");
	doc.Save(outputFilepath);
}

int main(int argc, char *argv[]) {
  
	const int k = 5;
	const int maxEpochs = 200;

  #ifdef MPI_TARGET
	boost::mpi::environment env(argc, argv); //MPI_Init
	boost::mpi::communicator world; //provides .rank() and .size()
  #endif
  
	#ifdef OMP_TARGET
  int thread_count = 5;
	#endif
	#ifdef CUDA_TARGET
	int block_size = 32;
	#endif

  std::string input_file = "data/spotify_short.csv";
  const std::string OUTPUT_FILE = "data/spotify_clusters.csv";
  if (argc >= 2) {
    input_file = argv[1];
  }
  if (argc >= 3) {
		#ifdef OMP_TARGET
		thread_count = std::stoi(argv[2]);
		#endif
		#ifdef CUDA_TARGET
		 block_size = std::stoi(argv[2]);
		#endif
  }
	#ifdef OMP_TARGET
	omp_set_num_threads(thread_count);
	#endif
	std::vector<Point> points;

  #ifdef MPI_TARGET
	if (world.rank() == 0) {
		std::cout << "Reading input data..." << std::endl;
		points = readInputData(input_file);
		std::cout << "Done. " << points.size() << " points loaded." << std::endl;
	}
  #endif
  #ifndef MPI_TARGET
  std::cout << "Reading input data..." << std::endl;
  points = readInputData(input_file);
  std::cout << "Done. " << points.size() << " points loaded." << std::endl;
  #endif
  
  #ifdef MPI_TARGET
	determineClusters(world, &points, k, maxEpochs);
  #endif

  #ifdef OMP_TARGET
  kMeansCluster(&points, maxEpochs, k);
  #endif

	#ifdef SERIAL_TARGET
  kMeansCluster(&points, maxEpochs, k);
  #endif

	#ifdef CUDA_TARGET
	kMeansCluster(&points, maxEpochs, k, block_size);
	#endif

  #ifdef MPI_TARGET
	if (world.rank() == 0) {
		//write output to csv
		std::cout << "Writing output csv..." << std::endl;
		writeClusterData(input_file, OUTPUT_FILE, &points);
		std::cout << "Done. See " << OUTPUT_FILE << std::endl;
	}
  #endif
  #ifndef MPI_TARGET
  //write output to csv
  std::cout << "Writing output csv..." << std::endl;
  writeClusterData(input_file, OUTPUT_FILE, &points);
  std::cout << "Done. See " << OUTPUT_FILE << std::endl;
  #endif

	return 0;

}
