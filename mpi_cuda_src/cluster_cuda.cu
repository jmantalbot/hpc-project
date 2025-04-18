#include <vector>
#include <float.h>
#include <cmath>
#include <cuda_runtime.h>
#include "point.hpp"
#include "cluster.hpp"
#define BLOCK_SIZE 256

__global__ void computeLocalPointCenters(Point **localPoints, bool* localChanged, float *centroidCoordinates, int clusterId, int dimensions) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    Point point = *localPoints[idx];
    float distance = 0.0;
    for(size_t i = 0; i < dimensions; i++){
        distance += std::pow(centroidCoordinates[i] - *(point.coordinates + i), 2);
    }
    if (distance < point.minDistance) {
        //update the point's centroid (what cluster it belongs to)
        point.minDistance = distance;
        point.cluster = clusterId;
        *localChanged = true;
    }
}

void updateLocalPoints(
    std::vector<Point>* localPoints,
    std::vector<Point>* centroids,
    int numberOfCoordinates,
    int maxEpochs,
    int k
) {
    
    size_t num_points = localPoints->size();
    const int blockSize = BLOCK_SIZE;
    const int gridSize = (num_points + blockSize - 1) / blockSize;
    
    for (std::vector<Point>::iterator centroidIterator = centroids->begin(); centroidIterator != centroids->end(); centroidIterator++) {
            int clusterId = centroidIterator - centroids->begin();

            // The following is replaced by CUDA, which follows

            // for (std::vector<Point>::iterator pointIterator = localPoints->begin(); pointIterator != localPoints->end(); pointIterator++) {
            //     Point point = *pointIterator;
            //     double distance = centroidIterator->distance(point);
            //     if (distance < point.minDistance) {
            //         //update the point's centroid (what cluster it belongs to)
            //         point.minDistance = distance;
            //         point.cluster = clusterId;
            //         localChanged = true;
            //     }
            //     *pointIterator = point;
            //}

            Point *d_localpoints;
            bool *d_localchanged;

            cudaMalloc(&d_localpoints, (size_t) (localPoints->end() - localPoints->begin()));
            cudaMalloc(&d_localchanged, sizeof(bool));

            cudaMemcpy(d_localpoints, localPoints->data(), localPoints->end() - localPoints->begin(), cudaMemcpyHostToDevice);
            cudaMemset(d_localchanged, false, sizeof(bool));

            computeLocalPointCenters<<<gridSize, blockSize>>>(&d_localpoints, d_localchanged, &centroidIterator->coordinates[0], clusterId, centroidIterator->coordinates.size());

            cudaDeviceSynchronize();
        }
}