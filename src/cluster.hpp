#pragma once
#include <vector>
#include "point.hpp"

Point calc_centroids(const std::vector<Point>& cluster);
std::vector<std::vector<Point>> k_means_cluster(int k, std::vector<Point>& points);

