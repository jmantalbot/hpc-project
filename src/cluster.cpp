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

/* --- calcMinimumDistances ----
 * Calculates the minimum distance between the points and the closest centroid.
 * Updates the cluster of each point as necessary, if a different centroid is now the closest.
 * Args: 
 *   std::vector<Point>* points // in and out
 *   std::vector<Point>* centroids // in
 */
bool calcMinimumDistances(std::vector<Point>* points, std::vector<Point>* centroids) {
    bool changed = false;
    for (std::vector<Point>::iterator centroidIterator = centroids->begin(); centroidIterator != centroids->end(); centroidIterator++) {
        int clusterId = centroidIterator - centroids->begin();
        for (std::vector<Point>::iterator pointIterator = points->begin(); pointIterator != points->end(); pointIterator++) {
            Point point = *pointIterator;
            float distance = centroidIterator->distance(point);
            if (distance < point.minDistance) {
                //update the point's centroid (what cluster it belongs to)
                point.minDistance = distance;
                point.cluster = clusterId;
                changed = true;
            }
            *pointIterator = point;
        }
    }
    return changed;
}

/* --- moveCentroids ----
 * Based on the cluster each point belongs to, compute the k-means and reposition the centroids.
 * Args: 
 *   std::vector<Point>* points // in and out (minDistance will change)
 *   std::vector<Point>* centroids // in and out
 *   int k // in
 */
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
    }
    //Move centroids to the mean coordinate of the points in its cluster
    for (std::vector<Point>::iterator centroidIterator = centroids->begin(); centroidIterator != centroids->end(); centroidIterator++) {
        int clusterId = centroidIterator - centroids->begin();
        for (size_t d = 0; d < sums.size(); d++) {
            centroidIterator->coordinates[d] = sums[d][clusterId] / numberOfPointsInEachCluster[clusterId];
        }
    }
}

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

    //limit the number of epochs -- prevents infinite loops.
    for (int epoch = 0; epoch < maxEpochs; epoch++) {
        // compute the distance from each centroid to each point
        // update the point's cluster as necessary.
        bool changed = calcMinimumDistances(points, &centroids);
        moveCentroids(points, &centroids, k);
        if(changed == false){
            std::cout << "This algorithm ran " << epoch << " number of times" << std::endl;
            break;
        }
    }
}
