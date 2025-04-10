/*
Serial implementation based on the provided example/tutorial at
https://github.com/robertmartin8/RandomWalks/blob/master/kmeans.cpp
*/

#include <vector>
#include <float.h>
#include <cmath>
#include <stdexcept>
#include <cuda_runtime.h>
#include <iostream>
#include "point.hpp"
#include <iomanip>
#include "cluster.hpp"
#define BLOCK_SIZE 256



__global__
void computeDistances(float* coordinates, float* centroids, int* clusters, float* minDistances, int num_points, int k, int d, int* changed_count) {
   int idx = blockIdx.x * blockDim.x + threadIdx.x;
   if (idx >= num_points) return;
   float min_dist = FLT_MAX;
   int min_cluster = clusters[idx];
   for (int i = 0; i < k; i++){
      float dist = 0.0;
      for (int dim = 0; dim < d; dim++) {
         float diff = coordinates[idx * d + dim] - centroids[i * d + dim];
         dist += diff * diff;
      }
      dist = sqrtf(dist);
      if (dist < min_dist) {
         min_dist = dist;
         min_cluster = i;
      }
   }
   if(min_dist < minDistances[idx]) {
       minDistances[idx] = min_dist;
       if (clusters[idx] != min_cluster) {
           atomicAdd(changed_count, 1);
           clusters[idx] = min_cluster;
       }
   }
}


__global__
void computeSums(float* coordinates, int* clusters, float* sums, int* counts, int num_points, int k, int d) {
   int idx = blockIdx.x * blockDim.x + threadIdx.x;
   if(idx >= num_points) return;
   int cluster = clusters[idx];
   if (cluster < 0  || cluster >= k) return;
   atomicAdd(&counts[cluster],1);
   for (int dim = 0; dim < d; dim++) {
      atomicAdd(&sums[cluster * d + dim], coordinates[idx * d + dim]);
   }
}


__global__
void updateCentroids(float* centroids, float* sums, int* counts, int k, int d) {
   int cluster = blockIdx.x;
   int dim = threadIdx.x;
   if (cluster >= k || dim >= d) return;
   int count = counts[cluster];
   if (count > 0) {
       centroids[cluster * d + dim] = sums[cluster * d + dim] / count;
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
    if (points->empty() || k <= 0 || maxEpochs <= 0) return;
    size_t num_points = points->size();
    size_t d = points->at(0).coordinates.size();
    float* h_coordinates = new float[num_points * d];
    int* h_clusters = new int[num_points];
    float* h_minDistances = new float[num_points];
    for(size_t i = 0; i < num_points; i++) {
        for (size_t j = 0; j < d; j++) {
            h_coordinates[i * d + j] = points->at(i).coordinates[j];
        }
        h_clusters[i] = -1;
        h_minDistances[i] = FLT_MAX;
    }
    float* h_centroids = new float[k * d];
    std::srand(100);
    for (int i = 0; i < k; i++) {
        int rand_idx = rand() % num_points;
        for (size_t j = 0; j < d; j++) {
            h_centroids[i * d + j] = h_coordinates[rand_idx * d + j];
        }
    }
    float *d_coordinates, *d_centroids, *d_minDistances, *d_sums;
    int *d_clusters, *d_counts, *d_changed;

    cudaMalloc(&d_coordinates, num_points * d * sizeof(float));
    cudaMalloc(&d_centroids, k * d * sizeof(float));
    cudaMalloc(&d_clusters, num_points * sizeof(int));
    cudaMalloc(&d_minDistances, num_points * sizeof(float));
    cudaMalloc(&d_counts, k * sizeof(int));
    cudaMalloc(&d_sums, k * d * sizeof(float));
    cudaMalloc(&d_changed, sizeof(int));

    cudaMemcpy(d_coordinates, h_coordinates, num_points * d * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(d_centroids, h_centroids, k * d * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(d_clusters, h_clusters, num_points * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(d_minDistances, h_minDistances, num_points * sizeof(float), cudaMemcpyHostToDevice);

    const int blockSize = BLOCK_SIZE;
    const int gridSize = (num_points + blockSize - 1) / blockSize;
    int h_changed = -1;
    for (int epoch = 0; epoch < maxEpochs; epoch++) {
        std::cout << "OMP EPOCH " << epoch << " Centroids:\n";
        for (int i = 0; i < k; i++) {
                 std::cout << "  Centroid " << i << ": (";
                 for (size_t dim = 0; dim < d; dim++) {
                     std::cout << std::fixed << std::setprecision(6) << h_centroids[i * d + dim];
                     if (dim < d - 1) std::cout << ", ";
                 }
                 std::cout <<")\n";
        }
        std::cout<<std::endl;
        if (h_changed == 0) {
            std::cout << "This Algorithm ran " << epoch << " times." << std::endl;
            break;
        }
        cudaMemset(d_changed, 0, sizeof(int));
        computeDistances<<<gridSize, blockSize>>>(d_coordinates, d_centroids, d_clusters, d_minDistances, num_points, k, d, d_changed);
        cudaDeviceSynchronize();
        cudaMemcpy(&h_changed, d_changed, sizeof(int), cudaMemcpyDeviceToHost);
        cudaMemset(d_sums, 0, k * d * sizeof(float));
        cudaMemset(d_counts, 0, k * sizeof(int));

        computeSums<<<gridSize, blockSize>>>(d_coordinates, d_clusters, d_sums, d_counts, num_points, k, d);
        cudaDeviceSynchronize();
        updateCentroids<<<k,d>>>(d_centroids, d_sums, d_counts, k, d);
        cudaDeviceSynchronize();
        cudaMemcpy(h_centroids, d_centroids, k * d * sizeof(float), cudaMemcpyDeviceToHost);
    }
    cudaMemcpy(h_clusters, d_clusters, num_points * sizeof(int), cudaMemcpyDeviceToHost);
    for (size_t i = 0; i < num_points; i++){
        points->at(i).cluster = h_clusters[i];
    }
        for (int i = 0; i < k; i++) {
                 std::cout << "  Centroid " << i << ": (";
                 for (size_t dim = 0; dim < d; dim++) {
                     std::cout << std::fixed << std::setprecision(6) << h_centroids[i * d + dim];
                     if (dim < d - 1) std::cout << ", ";
                 }
                 std::cout <<")\n";
        }
        std::cout<<std::endl;


    delete[] h_coordinates;
    delete[] h_clusters;
    delete[] h_minDistances;
    delete[] h_centroids;
    cudaFree(d_coordinates);
    cudaFree(d_centroids);
    cudaFree(d_clusters);
    cudaFree(d_minDistances);
    cudaFree(d_counts);
    cudaFree(d_sums);
    cudaFree(d_changed);
}
