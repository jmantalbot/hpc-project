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
	return input_data;
}

void writeClusterData(std::string inputFilepath, std::string outputFilepath, std::vector<Point>* points) {
	std::vector<int> clusters;
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

	const int k = 5;
	const int maxEpochs = 200;

	const std::string INPUT_FILE = "data/spotify.csv";
	const std::string OUTPUT_FILE = "data/spotify_clusters.csv";
	std::vector<Point> points;
	if (world.rank() == 0) {
		std::cout << "Reading input data..." << std::endl;
		points = readInputData(INPUT_FILE);
		std::cout << "Done. " << points.size() << " points loaded." << std::endl;
	}

	determineClusters(world, &points, k, maxEpochs);

	if (world.rank() == 0) {
		//write output to csv
		std::cout << "Writing output csv..." << std::endl;
		writeClusterData(INPUT_FILE, OUTPUT_FILE, &points);
		std::cout << "Done. See " << OUTPUT_FILE << std::endl;
	}

	return 0;

}
