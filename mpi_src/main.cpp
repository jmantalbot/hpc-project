#include <iostream>
#include <string>
#include <vector>
#include "rapidcsv.h"
#include "point.hpp"
#include "cluster.hpp"
#include <map>
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/mpi/collectives.hpp>

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


	//TODO: MPI -- broadcast document, scatter rows to read?
	std::vector<Point> input_data;
	std::vector<std::string> columnNames = doc.GetColumnNames();
	std::vector<std::string> row;
	for (size_t row_idx = 0; row_idx < doc.GetRowCount(); row_idx++) {
		//for each row
		row = doc.GetRow<std::string>(row_idx);
		std::vector<float> coordinates;
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
		input_data.push_back(Point(coordinates));
	}
	//TODO: MPI -- gather coordinates into input_data
	return input_data;
}

void writeClusterData(std::string inputFilepath, std::string outputFilepath, std::vector<Point>* points) {
	std::vector<int> clusters;
	//TODO: MPI -- scatter points? gather into clusters?
	for (size_t i = 0; i < points->size(); i++) {
		clusters.push_back(points->at(i).cluster);
	}
	rapidcsv::Document doc(inputFilepath);
	doc.InsertColumn<int>(doc.GetColumnCount(), clusters, "cluster");
	doc.Save(outputFilepath);
	
}

int main(int argc, char *argv[]) {

	boost::mpi::environment env(argc, argv); //MPI_Init
	boost::mpi::communicator world; //provides .rank() and .size()

	const std::string INPUT_FILE = "data/spotify_short.csv";
	const std::string OUTPUT_FILE = "data/spotify_clusters.csv";
	std::vector<Point> points;
	if (world.rank() == 0) {
		std::cout << "Reading input data..." << std::endl;
		points = readInputData("data/spotify_short.csv");
		std::cout << "Done. " << points.size() << " points loaded." << std::endl;
	}

	// clustering!
	const int k = 5;
	const int maxEpochs = 200;
	if (world.rank() == 0) {
		std::cout << "Determining clusters with k = " << k << "..." << std::endl;
	}
	int numberOfPoints;
	int numberOfCoordinates;
	std::vector<Point> localPoints;
	std::vector<Point> centroids;

	if (world.rank() == 0) {
		//basic information -- reduce number of calls to size functions
		numberOfPoints = points.size();
		numberOfCoordinates = points.at(0).coordinates.size();
		//Check that all points have the same dimensions as the first point.
		for (int i = 1; i < numberOfPoints; i++) {
			if (points.at(i).coordinates.size() != numberOfCoordinates) {
					throw std::invalid_argument("k_means_cluster: All points must have the same dimension.");
			}
		}
		std::srand(100);
		for (int centroidIdx = 0; centroidIdx < k; centroidIdx++) {
			centroids.push_back(points.at(rand() % numberOfPoints));
		}
	}
	else {
		centroids.resize(k);
	}
	boost::mpi::broadcast(world, numberOfPoints, 0);
	boost::mpi::broadcast(world, numberOfCoordinates, 0);

	std::vector<int> localPointCounts(world.size(), numberOfPoints / world.size());
	localPointCounts[world.size() - 1] += numberOfPoints % world.size();
	localPoints.resize(localPointCounts[world.rank()]);
	boost::mpi::scatterv(
		world,
		points.data(), //in_values, scattered to other processes
		localPointCounts, //sizes -- konwn by calling process
		localPoints.data(),  //out_values, destination for scattered data
		0 // root process
	);

	// kMeansCluster(&localPoints, localPointCounts[world.rank()], maxEpochs, k);
	kMeansCluster(
		world,
		&localPoints,
		&centroids,
		numberOfCoordinates,
		maxEpochs,
		k
	);

	boost::mpi::gatherv(
		world, 
		localPoints, //in values -- local points to transmit to root
		points.data(), //out values -- all gathered points
		localPointCounts, // sizes of in values 
		0 // root process
	);
	
	
	if (world.rank() == 0) {
		std::cout << "Done." << std::endl;
	}

	if (world.rank() == 0) {
		//write output to csv
		std::cout << "Writing output csv..." << std::endl;
		writeClusterData(INPUT_FILE, OUTPUT_FILE, &points);
		std::cout << "Done. See " << OUTPUT_FILE << std::endl;
	}

	return 0;

}
