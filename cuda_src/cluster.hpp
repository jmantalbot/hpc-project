#pragma once
#include <vector>
#include "point.hpp"

/* --- kMeansCluster ----
 * Determine the clusters for the given data points
 * Args: 
 *   std::vector<Point>* points // in and out
 *   int maxEpochs // in
 *   int k // in
 */
void kMeansCluster(std::vector<Point>* points, int maxEpochs, int k, int blockSize);

