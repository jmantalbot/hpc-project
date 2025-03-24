/*
Serial implementation based on the provided example/tutorial at 
https://github.com/robertmartin8/RandomWalks/blob/master/kmeans.cpp
*/

#include <vector>
#include <float.h>
#include <cmath>
#include <stdexcept>
#include <iostream>

#include "point.hpp"
#include "cluster.hpp"

bool calcMinimumDistances(std::vector<Point>* points, std::vector<Point>* centroids) {
    bool noChange = true;
    for (std::vector<Point>::iterator centroidIterator = centroids->begin(); centroidIterator != centroids->end(); centroidIterator++) {
        int clusterId = centroidIterator - centroids->begin();
        for (std::vector<Point>::iterator pointIterator = points->begin(); pointIterator != points->end(); pointIterator++) {
            Point point = *pointIterator;
            float distance = centroidIterator->distance(point);
            if (distance < point.minDistance) {
                //update the point's centroid (what cluster it belongs to)
                //If this code is not execute in the current epoch, the points have converged.
                noChange = false;
                point.minDistance = distance;
                point.cluster = clusterId;
            }
            *pointIterator = point;
        }
    }
    return noChange;
}

void moveCentroids(std::vector<Point>* points, std::vector<Point>* centroids, int k) {
    //Create vectors to keep track of data needed to compute means
    std::vector<int> numberOfPointsInEachCluster(k, 0);
    std::vector<std::vector<float>> sums(points->at(0).coordinates.size());
    for (int j = 0; j < k; j++) {
        for (size_t d = 0; d < sums.size(); d++) {
            sums[d].push_back(0.0);
        }
    }

    //Compute means
    //Compute sum of coordinates per cluster for each dimension
    for (std::vector<Point>::iterator pointIterator = points->begin(); pointIterator != points->end(); pointIterator++) {
        int clusterId = pointIterator->cluster;
        numberOfPointsInEachCluster[clusterId] += 1;
        for (size_t d = 0; d < sums.size(); d++) {
            sums[d][clusterId] += pointIterator->coordinates[d];
        }
        pointIterator->minDistance = FLT_MAX;
    }
    //Move centroids to the mean coordinate of the points in its cluster
    for (std::vector<Point>::iterator centroidIterator = centroids->begin(); centroidIterator != centroids->end(); centroidIterator++) {
        int clusterId = centroidIterator - centroids->begin();
        for (size_t d = 0; d < sums.size(); d++) {
            centroidIterator->coordinates[d] = sums[d][clusterId] / numberOfPointsInEachCluster[clusterId];
        }
    }
}


void k_means_cluster(std::vector<Point>* points, int maxEpochs, int k){
    //bounds checking
    if (points->empty() || k <= 0 || maxEpochs <= 0) {
        return;
    }
    //basic information -- reduce number of calls to size functions
    int numberOfPoints = points->size();

    //Check that all points have the same dimensions as the first point.
    for (int i = 1; i < numberOfPoints; i++) {
        if (points->at(i).coordinates.size() != points->at(0).coordinates.size()) {
            throw std::invalid_argument("k_means_cluster: All points must have the same dimension.");
        }
    }

    //vector of centroids, set capacity to k.
    std::vector<Point> centroids;
    centroids.reserve(k);
    // randomly select k points to be where the centroids start
    std::srand(100); // for consistency
    for (int centroidIdx = 0; centroidIdx < k; centroidIdx++) {
        //set coordinate to that of a random point
        centroids.push_back(points->at(rand() % numberOfPoints)); 
    }

    bool converged = true;
    //limit the number of epochs -- prevents infinite loops.
    for (int epoch = 0; epoch < maxEpochs; epoch++) {
        // compute the distance from each centroid to each point
        // update the point's cluster as necessary.
        converged = calcMinimumDistances(points, &centroids);
        // if no changes to what clusters the points are in, break early.
        if (converged) {
            break;
        }
        moveCentroids(points, &centroids, k);
    }
}
