#pragma once
#include <vector>
#include "point.hpp"
#include <boost/mpi/communicator.hpp>
#include "cluster_cuda.hpp"

void determineClusters(
  boost::mpi::communicator world,
  std::vector<Point>* points,
  int k,
  int maxEpochs
);

/* --- kMeansCluster ----
 * Determine the clusters for the given data points
 */
void kMeansCluster(boost::mpi::communicator world,
  std::vector<Point>* localPoints,
  std::vector<Point>* centroids,
  int numberOfCoordinates,
  int maxEpochs, 
  int k
);

