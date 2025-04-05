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
void kMeansCluster(std::vector<Point>* points, int maxEpochs, int k){
    //bounds checking
    if (points->empty() || k <= 0 || maxEpochs <= 0) {
        return;
    }

    int numberOfPoints;
    bool localChanged, anyChanged;
    int numberOfCoordinates;
    std::vector<Point> localPoints;
    std::vector<Point> centroids;
    centroids.reserve(k);

    boost::mpi::environment env; //MPI_Init
    boost::mpi::communicator world; //provides .rank() and .size()

    if (world.rank() == 0) {
        //basic information -- reduce number of calls to size functions
        numberOfPoints = points->size();
        numberOfCoordinates = points->at(0).coordinates.size();
        //Check that all points have the same dimensions as the first point.
        for (int i = 1; i < numberOfPoints; i++) {
            if (points->at(i).coordinates.size() != numberOfCoordinates) {
                throw std::invalid_argument("k_means_cluster: All points must have the same dimension.");
            }
        }

        //vector of centroids, set capacity to k.
        // randomly select k points to be where the centroids start
        std::srand(100); // for consistency
        for (int centroidIdx = 0; centroidIdx < k; centroidIdx++) {
            //set coordinate to that of a random point
            centroids.push_back(points->at(rand() % numberOfPoints)); 
        }
    }

    boost::mpi::broadcast(world, numberOfPoints, 0);
    boost::mpi::broadcast(world, numberOfCoordinates, 0);
    boost::mpi::broadcast(world, k, 0);

    std::vector<int> localPointCounts(world.size(), numberOfPoints / world.size());
    // last process also gets remainder points
    localPointCounts[world.size() - 1] += numberOfPoints % world.size();
    localPoints.resize(localPointCounts[world.rank()]);
    boost::mpi::scatterv(
        world,
        points->data(), //in_values, scattered to other processes
        localPointCounts, //sizes -- konwn by calling process
        localPoints.data(),  //out_values, destination for scattered data
        0 // root process
    );
    
    std::vector<int> localNumberOfPointsInEachCluster(k, 0);
    std::vector<int> globalNumberOfPointsInEachCluster(k, 0);
    std::vector<std::vector<float>> localSums(k, std::vector<float>(numberOfCoordinates, 0.0));
    std::vector<std::vector<float>> globalSums(k, std::vector<float>(numberOfCoordinates, 0.0));

    //limit the number of epochs -- prevents infinite loops.
    for (int epoch = 0; epoch < maxEpochs; epoch++) {
        if (world.rank() == 0) {
            for (int clusterId = 0; clusterId < k; clusterId++) {
                for (int dimension = 0; dimension < numberOfCoordinates; dimension++) {
                    globalSums[clusterId][dimension] = 0.0;
                }
            }
        }
        //broadcast updated at start of each epoch.
        boost::mpi::broadcast(world, centroids.data(), k, 0);
        anyChanged = false;
        localChanged = false;
        for (std::vector<Point>::iterator centroidIterator = centroids.begin(); centroidIterator != centroids.end(); centroidIterator++) {
            int clusterId = centroidIterator - centroids.begin();
            for (std::vector<Point>::iterator pointIterator = localPoints.begin(); pointIterator != localPoints.end(); pointIterator++) {
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
        
        //reduce changed -- if all are false then break from loop for all processes.
        boost::mpi::all_reduce(world, localChanged, anyChanged, std::logical_or<bool>());
        if (!anyChanged) {
            break;
        }

        // reset localSums to 0 for all clusters and dimensions.
        // for each cluster
        for (int clusterId = 0; clusterId < k; clusterId++) {
            for (int dimension = 0; dimension < numberOfCoordinates; dimension++) {
                localSums[clusterId][dimension] = 0.0;
            }
        }

        //Compute means
        //Compute sum of coordinates per cluster for each dimension
        for (std::vector<Point>::iterator pointIterator = localPoints.begin(); pointIterator != localPoints.end(); pointIterator++) {
            int clusterId = pointIterator->cluster;
            localNumberOfPointsInEachCluster[clusterId] += 1;
            for (size_t d = 0; d < k; d++) {
                localSums[clusterId][d] += pointIterator->coordinates[d];
            }
        }

        //reduce localSums and localNumberOfPointsInEachCluster
        boost::mpi::reduce(world, localNumberOfPointsInEachCluster.data(), k, globalNumberOfPointsInEachCluster.data(), std::plus<int>(), 0);
        for (int clusterId = 0; clusterId < k; clusterId++) {
            boost::mpi::reduce(world, localSums[k].data(), numberOfCoordinates, globalSums[k].data(), std::plus<float>(), 0);
        }

        //root computes averages and moves centroids.
        if (world.rank() == 0) {
            //Move centroids to the mean coordinate of the points in its cluster
            for (std::vector<Point>::iterator centroidIterator = centroids.begin(); centroidIterator != centroids.end(); centroidIterator++) {
                int clusterId = centroidIterator - centroids.begin();
                for (size_t d = 0; d < globalSums.size(); d++) {
                    centroidIterator->coordinates[d] = globalSums[clusterId][d] / globalNumberOfPointsInEachCluster[clusterId];
                }
            }
        }
    }

    boost::mpi::gatherv(
        world, 
        localPoints, //in values -- local points to transmit to root
        points->data(), //out values -- all gathered points
        localPointCounts, // sizes of in values 
        0 // root process
    );

    // MPI_Finalize(); //not necessary with boost

}
