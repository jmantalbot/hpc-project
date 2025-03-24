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


void k_means_cluster(std::vector<Point>* points, int maxEpochs, int k){ 
    int numberOfPoints = points->size();
    if (numberOfPoints == 0) {
        return;
    }

    std::vector<Point> centroids;
    // randomly select k points to be where the centroids start
    std::srand(100); // for consistency
    for (int centroidIdx = 0; centroidIdx < k; centroidIdx++) {
        centroids.push_back(points->at(rand() % numberOfPoints)); 
    }
    for (int epoch = 0; epoch < maxEpochs; epoch++) {
        // compute the distance from each centroid to each point
        // update the point's cluster as necessary. 
        
        for (std::vector<Point>::iterator centroidIterator = begin(centroids); centroidIterator != end(centroids); centroidIterator++) {
            int clusterId = centroidIterator - begin(centroids);
            for (std::vector<Point>::iterator pointIterator = points->begin(); pointIterator != points->end(); pointIterator++) {
                Point point = *pointIterator;
                float distance = centroidIterator->distance(point);
                if (distance < point.minDistance) {
                    point.minDistance = distance;
                    point.cluster = clusterId;
                }
                *pointIterator = point;
            }
        }
        
        //Create vectors to keep track of data eneded to compute means
        std::vector<int> numberOfPointsInEachCluster(k, 0);
        std::vector<std::vector<float>> sums(points->at(0).coordinates.size());
        for (int j = 0; j < k; j++) {
            for (int d = 0; d < sums.size(); d++) {
                sums[d].push_back(0.0);
            }
        }
        
        for (std::vector<Point>::iterator pointIterator = points->begin(); pointIterator != points->end(); pointIterator++) {
            int clusterId = pointIterator->cluster;
            numberOfPointsInEachCluster[clusterId] += 1;
            for (int d = 0; d < sums.size(); d++) {
                sums[d][clusterId] += pointIterator->coordinates[d];
            }
            pointIterator->minDistance = FLT_MAX;
        }
        for (std::vector<Point>::iterator centroidIterator = begin(centroids); centroidIterator != end(centroids); centroidIterator++) {
            int clusterId = centroidIterator - begin(centroids);
            for (int d = 0; d < sums.size(); d++) {
                centroidIterator->coordinates[d] = sums[d][clusterId] / numberOfPointsInEachCluster[clusterId];
            }
        }
    }
}
