#pragma once
#include <vector>
#include "point.hpp"
#include <boost/mpi/communicator.hpp>

/* --- kMeansCluster ----
 * Determine the clusters for the given data points
 * Args: 
 *   std::vector<Point>* points // in and out
 *   int maxEpochs // in
 *   int k // in
 */
void kMeansCluster(boost::mpi::communicator world,
  std::vector<Point>* localPoints,
  std::vector<Point>* centroids,
  int numberOfCoordinates,
  int maxEpochs, 
  int k
);

