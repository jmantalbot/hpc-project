/*
Serial implementation based on the provided example/tutorial at 
https://github.com/robertmartin8/RandomWalks/blob/master/kmeans.cpp
*/

#include <vector>
#include <float.h>
#include <cmath>
#include <stdexcept>
#include <iostream>
#include <omp.h>
#include "point.hpp"
#include "cluster.hpp"

/* --- calcMinimumDistances ----
 * Calculates the minimum distance between the points and the closest centroid.
 * Updates the cluster of each point as necessary, if a different centroid is now the closest.
 * Args:
 *   std::vector<Point>* points // in
 *   std::vector<Point>* centroids // in
 * Return: if any points have changed which centroid they are associated with.
 */
bool calcMinimumDistances(std::vector<Point>* points, std::vector<Point>* centroids) {
    bool changed = false;
    const int num_centroids = static_cast<int>(centroids->size());
/*ADDING THIS PARALLEL FOR CHANGES THE NUMBER OF ITERATIONS FROM A STABLE 3, to EITHER 3 OR 4*/
/*Additonally, the output was inconsistent for this for-loop with pragma for*/
    for (int centroid_idx = 0; centroid_idx < num_centroids; centroid_idx++) {
        const Point& centroid = centroids->at(centroid_idx);
        const int clusterId = centroid_idx;
        #pragma omp parallel for reduction(||:changed)
        for (int point_idx = 0; point_idx < static_cast<int>(points->size()); point_idx++) {
            Point& point = points->at(point_idx);
            const float distance = centroid.distance(point);
            if (distance < point.minDistance) {
                //update the point's centroid (what cluster it belongs to)
                {
                   point.minDistance = distance;
                   point.cluster = clusterId;
                   changed = true;
                }
            }
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
    std::vector<std::vector<float>> sums(points->at(0).coordinates.size(), std::vector<float>(k));

    //Compute means
    //Compute sum of coordinates per cluster for each dimension
    // This loop is likely unparallelizable using OMP
    for(const auto& point: *points){
        int clusterId = point.cluster;
        numberOfPointsInEachCluster[clusterId]++;
        for (size_t d = 0; d < sums.size(); d++) {
            sums[d][clusterId] += point.coordinates[d];
        }
    }
    //Move centroids to the mean coordinate of the points in its cluster
    #pragma omp parallel for
    for (std::vector<Point>::iterator centroidIterator = centroids->begin(); centroidIterator != centroids->end(); centroidIterator++) {
        int clusterId = centroidIterator - centroids->begin();
        #pragma omp parallel for
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
    //vector of centroids, set capacity to k.
    std::vector<Point> centroids(k);
    // randomly select k points to be where the centroids start
    std::srand(100); // for consistency
/*This loop cannot be parallelized and remain consistent.*/
    for (int i=0;i<k;i++) {
        //set coordinate to that of a random point
        centroids[i] = (points->at(rand() % numberOfPoints));
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
