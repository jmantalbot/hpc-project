#include <iostream>
#include <string>
#include <vector>
#include "rapidcsv.h"
#include "point.hpp"
#include "cluster.hpp"
#include <map>

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
	const std::string INPUT_FILE = "data/spotify_short.csv";
	const std::string OUTPUT_FILE = "data/spotify_clusters.csv";

	std::cout << "Reading input data..." << std::endl;
	std::vector<Point> points = readInputData("data/spotify_short.csv");
	std::cout << "Done. " << points.size() << " points loaded." << std::endl;

	// clustering!
	const int k = 5;
	const int maxEpochs = 200;
	std::cout << "Determining clusters with k = " << k << "..." << std::endl;
	kMeansCluster(&points, maxEpochs, k);
	std::cout << "Done." << std::endl;

	//write output to csv
	std::cout << "Writing output csv..." << std::endl;
	writeClusterData(INPUT_FILE, OUTPUT_FILE, &points);
	std::cout << "Done. See " << OUTPUT_FILE << std::endl;

	//doc.InsertColumn
	//doc.Save

	//profit

	return 0;

}
