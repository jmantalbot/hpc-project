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
void calcMinimumDistances(std::vector<Point>* points, std::vector<Point>* centroids, std::vector<std::vector<Point>> clusters) {
    Point point;
    for (std::vector<Point>::iterator centroidIterator = centroids->begin(); centroidIterator != centroids->end(); centroidIterator++) {
        int clusterId = centroidIterator - centroids->begin();
	int closest = 0;
        for (std::vector<Point>::iterator pointIterator = points->begin(); pointIterator != points->end(); pointIterator++) {
            point = *pointIterator;
            float distance = centroidIterator->distance(point);
            if (distance < point.minDistance) {
                //update the point's centroid (what cluster it belongs to)
                point.minDistance = distance;
                point.cluster = clusterId;
		closest = clusterId;
            }
            *pointIterator = point;
        }
	clusters[closest].push_back(point);
    }
}

/* --- moveCentroids ----
 * Based on the cluster each point belongs to, compute the k-means and reposition the centroids.
 * Args: 
 *   std::vector<Point>* points // in and out (minDistance will change)
 *   std::vector<Point>* centroids // in and out
 *   int k // in
 */
Point moveCentroids(const std::vector<Point>& cluster){
    if(cluster.empty()) return Point();
    std::vector<float> sums(cluster[0].coordinates.size(), 0.0);
    for(const auto& point: cluster) {
        for(size_t i=0; i<sums.size(); i++){
            sums[i] += point.coordinates[i];
        }
    }
    for(auto& sum:sums){
        sum /= cluster.size();
    }
    Point c =  Point(sums);
    return c;
}




/* --- kMeansCluster ----
 * Determine the clusters for the given data points
 * Args: 
 *   std::vector<Point>* points // in and out
 *   int maxEpochs // in
 *   int k // in
 */
void kMeansCluster(std::vector<Point>* points, int k){
    //bounds checking
    if (points->empty() || k <= 0) {
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
    std::vector<std::vector<Point>> clusters(k);
    bool converged = false;
    while(!converged){
        for(auto& cluster:clusters){
            cluster.clear();
            }
        calcMinimumDistances(points, &centroids, clusters);
        std::vector<Point> newCentroids(k);
        for (int i= 0; i<k;i++){
            newCentroids[i] = clusters[i].empty() ? centroids[i] : moveCentroids(clusters[i]);
        }
        converged = (newCentroids == centroids);
        centroids = newCentroids;
    }
}
