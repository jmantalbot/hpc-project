#include <iostream>
#include <string>
#include <vector>
#include "rapidcsv.h"
#include "point.hpp"
#include <map>

int main(int argc, char *argv[]) {

	std::cout << "Testing clustering..." << std::endl;
	
	rapidcsv::Document doc("data/spotify_short.csv");

	//run mock data through clustering algorithm

	// for now, ignoring id,name,album,album_id,artists,artist_ids,track_number,disc_number,explicit,duration_ms,year,release_date
	std::vector<std::string> feature_keys = {
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
		//get explicit,danceability,energy,key,loudness,mode,speechiness,acousticness,instrumentalness,liveness,valence,tempo,duration_ms,time_signature
		
		std::vector<float> coordinates;
		for (size_t feature_idx = 0; feature_idx < feature_keys.size(); feature_idx++) {
			std::string feature = doc.GetCell<std::string>(feature_keys[feature_idx], row_idx);
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

		// for (int i = 0; i < coordinates.size(); i++) {
		// 	std::cout << coordinates[i] << std::endl;
		// }

	}
	//testing -- print the first row
	std::cout << input_data[0].toString() << std::endl;
	std::cout << input_data[499].toString() << std::endl;

	//write output to csv

	//doc.InsertColumn
	//doc.Save

	//profit

	return 0;

}
