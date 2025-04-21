#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <cuda_runtime.h>
#include "cluster.h"

#define BLOCK_SIZE 256

__device__ float distance(const float* a, const float* b, int D) {
    float dist = 0.0f;
    for (int i = 0; i < D; ++i) {
        float diff = a[i] - b[i];
        dist += diff * diff;
    }
    return sqrtf(dist);
}

__global__ void computeAssignments(
    const float* coordinates,   // [num_points * D]
    const float* centroids,     // [k * D]
    int* assignments,           // [num_points]
    float* minDistances,        // [num_points]
    int* changed_count,
    int num_points,
    int k,
    int D
) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= num_points) return;

    const float* point = &coordinates[idx * D];
    float min_dist = FLT_MAX;
    int min_cluster = -1;

    for (int i = 0; i < k; ++i) {
        const float* centroid = &centroids[i * D];
        float dist = distance(point, centroid, D);
        if (dist < min_dist) {
            min_dist = dist;
            min_cluster = i;
        }
    }

    if (assignments[idx] != min_cluster) {
        atomicAdd(changed_count, 1);
        assignments[idx] = min_cluster;
    }

    minDistances[idx] = min_dist;
}

__global__ void accumulateCentroids(
    const float* coordinates,
    const int* assignments,
    float* sums,
    int* counts,
    int num_points,
    int k,
    int D
) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= num_points) return;

    int cluster = assignments[idx];
    if (cluster < 0 || cluster >= k) return;

    for (int d = 0; d < D; ++d) {
        atomicAdd(&sums[cluster * D + d], coordinates[idx * D + d]);
    }
    atomicAdd(&counts[cluster], 1);
}

__global__ void updateCentroids(
    float* centroids,
    const float* sums,
    const int* counts,
    int k,
    int D
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= k) return;

    int count = counts[i];
    if (count == 0) return;

    for (int d = 0; d < D; ++d) {
        centroids[i * D + d] = sums[i * D + d] / count;
    }
}

void run_kmeans_gpu(float* coords, int* assignments, float* centroids, int num_points, int k, int maxEpochs, int D) {
    float *d_coords, *d_centroids, *d_sums, *d_minDistances;
    int *d_assignments, *d_counts, *d_changed;

    size_t coord_size = num_points * D * sizeof(float);
    size_t centroid_size = k * D * sizeof(float);

    cudaMalloc(&d_coords, coord_size);
    cudaMalloc(&d_centroids, centroid_size);
    cudaMalloc(&d_assignments, num_points * sizeof(int));
    cudaMalloc(&d_minDistances, num_points * sizeof(float));
    cudaMalloc(&d_counts, k * sizeof(int));
    cudaMalloc(&d_sums, centroid_size);
    cudaMalloc(&d_changed, sizeof(int));

    cudaMemcpy(d_coords, coords, coord_size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_centroids, centroids, centroid_size, cudaMemcpyHostToDevice);
    cudaMemset(d_assignments, -1, num_points * sizeof(int));

    int h_changed = 1;
    int threads = BLOCK_SIZE;
    int blocks_points = (num_points + threads - 1) / threads;
    int blocks_centroids = (k + threads - 1) / threads;

    for (int epoch = 0; epoch < maxEpochs && h_changed > 0; epoch++) {
        cudaMemset(d_changed, 0, sizeof(int));

        computeAssignments<<<blocks_points, threads>>>(
            d_coords, d_centroids, d_assignments, d_minDistances, d_changed,
            num_points, k, D);
        cudaDeviceSynchronize();

        cudaMemcpy(&h_changed, d_changed, sizeof(int), cudaMemcpyDeviceToHost);

        cudaMemset(d_sums, 0, centroid_size);
        cudaMemset(d_counts, 0, k * sizeof(int));

        accumulateCentroids<<<blocks_points, threads>>>(
            d_coords, d_assignments, d_sums, d_counts,
            num_points, k, D);
        cudaDeviceSynchronize();

        updateCentroids<<<blocks_centroids, threads>>>(
            d_centroids, d_sums, d_counts, k, D);
        cudaDeviceSynchronize();
    }

    cudaMemcpy(assignments, d_assignments, num_points * sizeof(int), cudaMemcpyDeviceToHost);
    cudaMemcpy(centroids, d_centroids, centroid_size, cudaMemcpyDeviceToHost);

    cudaFree(d_coords);
    cudaFree(d_centroids);
    cudaFree(d_assignments);
    cudaFree(d_minDistances);
    cudaFree(d_counts);
    cudaFree(d_sums);
    cudaFree(d_changed);
}
