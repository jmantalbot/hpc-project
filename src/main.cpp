#include <iostream>
#include <string>
#include <vector>
#include "rapidcsv.h"
#include "point.hpp"


int main(int argc, char *argv[]) {

	std::cout << "Testing clustering..." << std::endl;
	
	rapidcsv::Document doc("data/spotify_short.csv");

	//run mock data through clustering algorithm

	std::vector<Point> input_data(doc.GetRowCount());
	std::vector<std::string> columnNames = doc.GetColumnNames();
	std::vector<std::string> row;
	for (size_t i = 0; i < doc.GetRowCount(); i++) {
		//for each row
		row = doc.GetRow<std::string>(i);

		//get explicit,danceability,energy,key,loudness,mode,speechiness,acousticness,instrumentalness,liveness,valence,tempo,duration_ms,time_signature

	}

	//testing -- print the first row
	std::cout << "first row: ";
	for (int i = 0; i < row.size(); i++) {
		std::cout << row[i] << ", ";
	}
	std::cout << std::endl;

	//write output to csv

	//doc.InsertColumn
	//doc.Save

	//profit

	return 0;

}
