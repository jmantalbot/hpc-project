/*
Serial implementation based on the provided example/tutorial at 
https://github.com/robertmartin8/RandomWalks/blob/master/kmeans.cpp
*/

#include <vector>
#include <float.h>
#include <cmath>
#include <stdexcept>
#include <iostream>
#include <functional>
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/mpi/collectives.hpp>
#include "point.hpp"
#include "cluster.hpp"
#include <iomanip>

void determineClusters(
    boost::mpi::communicator world,
    std::vector<Point>* points,
    int k,
    int maxEpochs
) {
	if (world.rank() == 0) {
		std::cout << "Determining clusters with k = " << k << "..." << std::endl;
	}
	int numberOfPoints;
	int numberOfCoordinates;
	std::vector<Point> localPoints;
	std::vector<Point> centroids;

    if (world.rank() == 0) {
        numberOfPoints = points->size();
        numberOfCoordinates = points->at(0).coordinates.size();
    }
	boost::mpi::broadcast(world, numberOfPoints, 0);
	boost::mpi::broadcast(world, numberOfCoordinates, 0);

    // initialize centroids
    if (world.rank() == 0) {
		std::srand(100);
		for (int centroidIdx = 0; centroidIdx < k; centroidIdx++) {
			centroids.push_back(points->at(rand() % points->size()));
		}
    }
    else {
	centroids.resize(k);
    }
    //distribute points among the processes
	std::vector<int> localPointCounts(world.size(), numberOfPoints / world.size());
	localPointCounts[world.size() - 1] += numberOfPoints % world.size();
	localPoints.resize(localPointCounts[world.rank()]);
	boost::mpi::scatterv(
		world,
		points->data(), //in_values, scattered to other processes
		localPointCounts, //sizes -- konwn by calling process
		localPoints.data(),  //out_values, destination for scattered data
		0 // root process
	);

	kMeansCluster(
		world,
		&localPoints,
		&centroids,
		numberOfCoordinates,
		maxEpochs,
		k
	);

    //gather points from the processes
	boost::mpi::gatherv(
		world,
		localPoints, //in values -- local points to transmit to root
		points->data(), //out values -- all gathered points
		localPointCounts, // sizes of in values
		0 // root process
	);

	if (world.rank() == 0) {
		std::cout << "Done." << std::endl;
	}
}

/* --- kMeansCluster ----
 * Determine the clusters for the given data points
 */
void kMeansCluster(
    boost::mpi::communicator world,
    std::vector<Point>* localPoints,
    std::vector<Point>* centroids,
    int numberOfCoordinates,
    int maxEpochs,
    int k
){

    bool localChanged, anyChanged;
    std::vector<int> localNumberOfPointsInEachCluster(k, 0);
    std::vector<int> globalNumberOfPointsInEachCluster(k, 0);
    std::vector<std::vector<double>> localSums(k, std::vector<double>(numberOfCoordinates, 0.0));
    std::vector<std::vector<double>> globalSums(k, std::vector<double>(numberOfCoordinates, 0.0));

    //limit the number of epochs -- prevents infinite loops.
    for (int epoch = 0; epoch < maxEpochs; epoch++) {
        //broadcast updated centroids at start of each epoch.
        boost::mpi::broadcast(world, centroids->data(), k, 0);

        // Reassign each point to the nearest centroid's cluster. Break if no changes are made globally.
        anyChanged = false;
        localChanged = false;
        
        updateLocalPoints(localPoints, centroids, numberOfCoordinates, maxEpochs, k);

        //reduce changed -- if all are false then break from loop for all processes.
        boost::mpi::all_reduce(world, localChanged, anyChanged, std::logical_or<bool>());
        if (!anyChanged) {
            if (world.rank() == 0) {
                std::cout << "Converged after " << epoch << " iterations." << std::endl;
            }
            break;
        }

        // reset globalSums to 0 for all clusters and dimensions
        if (world.rank() == 0) {
            for (int clusterId = 0; clusterId < k; clusterId++) {
                for (int dimension = 0; dimension < numberOfCoordinates; dimension++) {
                    globalSums[clusterId][dimension] = 0.0;
                }
                globalNumberOfPointsInEachCluster[clusterId] = 0;
            }
        }
        // reset localSums to 0 for all clusters and dimensions.
        for (int clusterId = 0; clusterId < k; clusterId++) {
            for (int dimension = 0; dimension < numberOfCoordinates; dimension++) {
                localSums[clusterId][dimension] = 0.0;
            }
            localNumberOfPointsInEachCluster[clusterId] = 0;
        }

        // Changed by CUDA implementation!

        //Compute means
        //Compute sum of coordinates per cluster for each dimension
        // for (std::vector<Point>::iterator pointIterator = localPoints->begin(); pointIterator != localPoints->end(); pointIterator++) {
        //     int clusterId = pointIterator->cluster;
        //     localNumberOfPointsInEachCluster[clusterId] += 1;
        //     for (int d = 0; d < numberOfCoordinates; d++) {
        //         localSums[clusterId][d] += pointIterator->coordinates[d];
        //     }
        // }

        //reduce localSums and localNumberOfPointsInEachCluster
        boost::mpi::reduce(world, localNumberOfPointsInEachCluster.data(), k, globalNumberOfPointsInEachCluster.data(), std::plus<int>(), 0);
        for (int clusterId = 0; clusterId < k; clusterId++) {
            boost::mpi::reduce(world, localSums[clusterId].data(), numberOfCoordinates, globalSums[clusterId].data(), std::plus<float>(), 0);
        }

        //root computes averages given sums and moves centroids
        if (world.rank() == 0) {
            //Move centroids to the mean coordinate of the points in its cluster
            for (std::vector<Point>::iterator centroidIterator = centroids->begin(); centroidIterator != centroids->end(); centroidIterator++) {
                int clusterId = centroidIterator - centroids->begin();
                for (int d = 0; d < numberOfCoordinates; d++) {
                    centroidIterator->coordinates[d] = globalSums[clusterId][d] / globalNumberOfPointsInEachCluster[clusterId];
                }
            }
        }
    }
}
