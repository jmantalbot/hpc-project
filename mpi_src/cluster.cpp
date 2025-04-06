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

/* --- kMeansCluster ----
 * Determine the clusters for the given data points
 * Args: 
 *   std::vector<Point>* points // in and out
 *   int maxEpochs // in
 *   int k // in
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
    std::vector<std::vector<float>> localSums(k, std::vector<float>(numberOfCoordinates, 0.0));
    std::vector<std::vector<float>> globalSums(k, std::vector<float>(numberOfCoordinates, 0.0));

    std::cout << "here 1! " << world.rank() << std::endl;
    //limit the number of epochs -- prevents infinite loops.
    for (int epoch = 0; epoch < maxEpochs; epoch++) {
        std::cout << "here 2! " << world.rank() << std::endl;
        if (world.rank() == 0) {
            for (int clusterId = 0; clusterId < k; clusterId++) {
                for (int dimension = 0; dimension < numberOfCoordinates; dimension++) {
                    globalSums[clusterId][dimension] = 0.0;
                }
            }
        }
        //broadcast updated at start of each epoch.
        boost::mpi::broadcast(world, centroids->data(), k, 0);
        std::cout << "here 3! " << world.rank() << std::endl;
        anyChanged = false;
        localChanged = false;
        for (std::vector<Point>::iterator centroidIterator = centroids->begin(); centroidIterator != centroids->end(); centroidIterator++) {
            int clusterId = centroidIterator - centroids->begin();
            for (std::vector<Point>::iterator pointIterator = localPoints->begin(); pointIterator != localPoints->end(); pointIterator++) {
                Point point = *pointIterator;
                float distance = centroidIterator->distance(point);
                if (distance < point.minDistance) {
                    //update the point's centroid (what cluster it belongs to)
                    point.minDistance = distance;
                    point.cluster = clusterId;
                    localChanged = true;
                }
                *pointIterator = point;
            }
        }
        std::cout << "here 4! " << world.rank() << std::endl;

        //reduce changed -- if all are false then break from loop for all processes.
        boost::mpi::all_reduce(world, localChanged, anyChanged, std::logical_or<bool>());
        if (!anyChanged) {
            std::cout << "breaking after " << epoch << " iterations." << std::endl;
            break;
        }
        std::cout << "here 5! " << world.rank() << std::endl;

        // reset localSums to 0 for all clusters and dimensions.
        // for each cluster
        for (int clusterId = 0; clusterId < k; clusterId++) {
            for (int dimension = 0; dimension < numberOfCoordinates; dimension++) {
                localSums[clusterId][dimension] = 0.0;
            }
        }
        std::cout << "here 6! " << world.rank() << std::endl;

        //Compute means
        //Compute sum of coordinates per cluster for each dimension
        for (std::vector<Point>::iterator pointIterator = localPoints->begin(); pointIterator != localPoints->end(); pointIterator++) {
            int clusterId = pointIterator->cluster;
            localNumberOfPointsInEachCluster[clusterId] += 1;
            for (size_t d = 0; d < k; d++) {
                localSums[clusterId][d] += pointIterator->coordinates[d];
            }
        }
        std::cout << "here 7! " << world.rank() << std::endl;

        //reduce localSums and localNumberOfPointsInEachCluster
        boost::mpi::reduce(world, localNumberOfPointsInEachCluster.data(), k, globalNumberOfPointsInEachCluster.data(), std::plus<int>(), 0);
        for (int clusterId = 0; clusterId < k; clusterId++) {
            std::cout << "here 7.5! " << world.rank() << std::endl;
            boost::mpi::reduce(world, localSums[clusterId].data(), numberOfCoordinates, globalSums[clusterId].data(), std::plus<float>(), 0);
        }
        std::cout << "here 8! " << world.rank() << std::endl;

        //root computes averages and moves centroids->
        if (world.rank() == 0) {
            //Move centroids to the mean coordinate of the points in its cluster
            for (std::vector<Point>::iterator centroidIterator = centroids->begin(); centroidIterator != centroids->end(); centroidIterator++) {
                int clusterId = centroidIterator - centroids->begin();
                for (size_t d = 0; d < globalSums.size(); d++) {
                    centroidIterator->coordinates[d] = globalSums[clusterId][d] / globalNumberOfPointsInEachCluster[clusterId];
                }
            }
        }
        std::cout << "here 9! " << world.rank() << std::endl;
        
    }

    // MPI_Finalize(); //not necessary with boost

}
