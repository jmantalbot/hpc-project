// cluster.h
#ifndef CLUSTER_H
#define CLUSTER_H

#ifdef __cplusplus
extern "C" {
#endif

void run_kmeans_gpu(float* coords, int* assignments, float* centroids, int num_points, int k, int maxEpochs, int D);

#ifdef __cplusplus
}
#endif

#endif
