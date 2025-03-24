#include <iostream>
#include <string>
#include <vector>
#include "rapidcsv.h"
#include "point.hpp"
#include "cluster.hpp"
#include <map>

std::vector<Point> readInputData(std::string filepath) {
	rapidcsv::Document doc("data/spotify_short.csv");

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

int main(int argc, char *argv[]) {

	std::cout << "Reading input data..." << std::endl;
	std::vector<Point> input_data = readInputData("data/spotify_short.csv");
	std::cout << "Done." << std::endl;

	// clustering!
	const int k = 2;
	std::cout << "Determining clusters with k = " << k << "..." << std::endl;
	std::vector<std::vector<Point>> clusters = k_means_cluster(k, input_data);
	std::cout << "Done." << std::endl;

	for (int i = 0; i < clusters.size(); i++) {
		for (int j = 0; j < clusters[i].size(); j++) {
			std::cout << clusters[i][j].toString() << " -- ";
		}
		std::cout << std::endl << std::endl;
	}
	//write output to csv

	//doc.InsertColumn
	//doc.Save

	//profit

	return 0;

}
