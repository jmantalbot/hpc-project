#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <cuda_runtime.h>
#include "cluster.h"

#define D 3  // x, y, z
#define BLOCK_SIZE 256

struct point {
    int line_number;
    float x;
    float y;
    float z;
};

// Compute Euclidean distance between point and centroid
__device__ float distance(float* coords1, float* coords2) {
    float dx = coords1[0] - coords2[0];
    float dy = coords1[1] - coords2[1];
    float dz = coords1[2] - coords2[2];
    return sqrtf(dx * dx + dy * dy + dz * dz);
}

// Assign points to the closest centroid
__global__ void computeAssignments(
    float* coordinates,       // [num_points * 3]
    float* centroids,         // [k * 3]
    int* assignments,         // [num_points]
    float* minDistances,      // [num_points]
    int* changed_count,       // device-side counter
    int num_points,
    int k
) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= num_points) return;

    float min_dist = FLT_MAX;
    int min_cluster = assignments[idx];

    float point[3] = {
        coordinates[idx * D],
        coordinates[idx * D + 1],
        coordinates[idx * D + 2]
    };

    for (int i = 0; i < k; i++) {
        float* centroid = &centroids[i * D];
        float dist = distance(point, centroid);
        if (dist < min_dist) {
            min_dist = dist;
            min_cluster = i;
        }
    }

    if (min_dist < minDistances[idx]) {
        minDistances[idx] = min_dist;
        if (assignments[idx] != min_cluster) {
            atomicAdd(changed_count, 1);
            assignments[idx] = min_cluster;
        }
    }
}

// Accumulate new centroid sums
__global__ void accumulateCentroids(
    float* coordinates,  // [num_points * 3]
    int* assignments,    // [num_points]
    float* sums,         // [k * 3]
    int* counts,         // [k]
    int num_points,
    int k
) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= num_points) return;

    int cluster = assignments[idx];
    if (cluster < 0 || cluster >= k) return;

    atomicAdd(&sums[cluster * D + 0], coordinates[idx * D + 0]);
    atomicAdd(&sums[cluster * D + 1], coordinates[idx * D + 1]);
    atomicAdd(&sums[cluster * D + 2], coordinates[idx * D + 2]);
    atomicAdd(&counts[cluster], 1);
}

// Compute new centroids
__global__ void updateCentroids(
    float* centroids,  // [k * 3]
    float* sums,       // [k * 3]
    int* counts,       // [k]
    int k
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= k) return;

    int count = counts[i];
    if (count > 0) {
        centroids[i * D + 0] = sums[i * D + 0] / count;
        centroids[i * D + 1] = sums[i * D + 1] / count;
        centroids[i * D + 2] = sums[i * D + 2] / count;
    }
}

void run_kmeans_gpu(float* coords, int* assignments, float* centroids, int num_points, int k, int maxEpochs) {
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
    cudaMemset(d_minDistances, 0x7F, num_points * sizeof(float)); // Set to FLT_MAX

    int h_changed = 1;
    int threads = BLOCK_SIZE;
    int blocks_points = (num_points + threads - 1) / threads;
    int blocks_centroids = (k + threads - 1) / threads;

    for (int epoch = 0; epoch < maxEpochs && h_changed > 0; epoch++) {
        cudaMemset(d_changed, 0, sizeof(int));
        computeAssignments<<<blocks_points, threads>>>(d_coords, d_centroids, d_assignments, d_minDistances, d_changed, num_points, k);
        cudaDeviceSynchronize();

        cudaMemcpy(&h_changed, d_changed, sizeof(int), cudaMemcpyDeviceToHost);

        cudaMemset(d_sums, 0, centroid_size);
        cudaMemset(d_counts, 0, k * sizeof(int));

        accumulateCentroids<<<blocks_points, threads>>>(d_coords, d_assignments, d_sums, d_counts, num_points, k);
        cudaDeviceSynchronize();

        updateCentroids<<<blocks_centroids, 1>>>(d_centroids, d_sums, d_counts, k);
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
