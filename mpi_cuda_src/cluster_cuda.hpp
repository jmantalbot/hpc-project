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
void updateLocalPoints(
    std::vector<Point>* localPoints,
    std::vector<Point>* centroids,
    int numberOfCoordinates,
    int maxEpochs,
    int k
);

