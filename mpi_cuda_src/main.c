#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "cluster.h"

#define BUF_SIZE 262144

#define DANCEABILITY_INDEX 9
#define ENERGY_INDEX 10
#define VALENCE_INDEX 18

#define D 3  // x, y, z

struct point {
    int line_number;
    float x, y, z;
};

struct point *localpoints;
struct point *centroids;
int num_points;
int k;

int cmp(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}

void setup_and_scatter_csv(int argc, char *argv[], int rank, int processes) {
    int lines = -1; // Don't count the header
    int lines_per_process;
    FILE *csvfp;
    csvfp = fopen(argv[1], "r");
    char buf[BUF_SIZE];
    int *centroid_indices = calloc(k, sizeof(int));

    if (!rank) {

        size_t read_bytes;

        while ((read_bytes = fread(buf, 1, BUF_SIZE, csvfp))) {
            if (ferror(csvfp)) {
                fprintf(stderr, "Error while reading from file!\n");
                exit(1);
            }
            
            for (int i = 0; i < read_bytes; i++) {
                if (buf[i] == '\n') lines++;
            }
        }

        printf("%s contains %d points\n", argv[1], lines);

        fseek(csvfp, 0, SEEK_SET);
    }

    MPI_Bcast(&lines, 1, MPI_INT, 0, MPI_COMM_WORLD);

    srand(100);

    for (int i = 0; i < k; i++) {
        centroid_indices[i] = rand() % lines;
    }
    qsort(centroid_indices, k, sizeof(int), cmp);

    lines_per_process = lines / processes;

    const int REMAINDER = lines % processes;

    int extra = (rank < REMAINDER) ? 1 : 0;
    int startline = 1; // Skip header

    for (int i = 0; i < rank; i++) {
        startline += lines / processes + (i < REMAINDER ? 1 : 0);
    }

    int displs = startline + lines / processes + extra;
    num_points = displs - startline;

    printf("process %d will read lines %d - %d\n", rank, startline, displs);

    if (!csvfp) {
        fprintf(stderr, "Process %d failed to open file\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int current_line = 0;
    int local_index = 0;

    localpoints = malloc(num_points * sizeof(struct point));
    int centroid_index = 0;

    centroids = malloc(k * sizeof(struct point));

    while (fgets(buf, BUF_SIZE, csvfp)) {
        // Break comma-separated string into tokens
        char *tokens[32];
        int col = 0;
        char *token = strtok(buf, ",");
        while (token && col < 32) {
            tokens[col++] = token;
            token = strtok(NULL, ",");
        }
        if (col < VALENCE_INDEX) {
            fprintf(stderr, "Process %d: Incorrect number of columns on line %d\n", rank, current_line);
        }

        if (current_line >= startline && current_line < displs) {
            struct point p;
            p.line_number = current_line + 1;
            p.x = strtof(tokens[DANCEABILITY_INDEX], NULL);
            p.y = strtof(tokens[ENERGY_INDEX], NULL);
            p.z = strtof(tokens[VALENCE_INDEX], NULL);
            localpoints[local_index++] = p;
        }

        if (centroid_index < k && centroid_indices[centroid_index] == current_line) { 
            struct point centroid;
            centroid.line_number = current_line + 1;
            centroid.x = strtof(tokens[DANCEABILITY_INDEX], NULL);
            centroid.y = strtof(tokens[ENERGY_INDEX], NULL);
            centroid.z = strtof(tokens[VALENCE_INDEX], NULL);
            centroids[centroid_index++] = centroid;
        }

        current_line++;
    }

    fclose(csvfp);
}

int main(int argc, char *argv[]) {
    int rank;
    int processes;

	int maxEpochs = 200;

    k = 5;
    maxEpochs = 200;
    if (argc > 2) k = strtol(argv[2], NULL, 10);
    if (argc > 3) maxEpochs = strtol(argv[3], NULL, 10);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &processes);

    setup_and_scatter_csv(argc, argv, rank, processes);

// Each MPI process now has the same list of centroids and its own list of points
    // Now, have each MPI process use a CUDA kernel to simulate an epoch
    float* coords = malloc(num_points * D * sizeof(float));
    int* assignments = malloc(num_points * sizeof(int));
    float* centroid_array = malloc(k * D * sizeof(float));

    for (int i = 0; i < num_points; i++) {
        coords[i * D + 0] = localpoints[i].x;
        coords[i * D + 1] = localpoints[i].y;
        coords[i * D + 2] = localpoints[i].z;
    }

    // Initialize centroids[] based on your global centroids
    for (int i = 0; i < k; i++) {
        centroid_array[i * D + 0] = centroids[i].x;
        centroid_array[i * D + 1] = centroids[i].y;
        centroid_array[i * D + 2] = centroids[i].z;
    }

    // Synchronize centroids across all processes using MPI_Allreduce
    // We use MPI_Allreduce to compute the global centroid for each cluster (mean of all centroids)
    float* global_centroids = malloc(k * D * sizeof(float));

    for (int epoch = 0; epoch < maxEpochs; epoch++) {

        // Run the CUDA-based K-means algorithm
        run_kmeans_gpu(coords, assignments, centroid_array, num_points, k, 1); // 1 epoch at a time

        MPI_Allreduce(centroid_array, global_centroids, k * D, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);

        // Now we divide by the number of processes to get the average centroid
        // TODO: THIS SHOULD BE A WEIGHTED AVERAGE USING THE NUMBER OF POINTS PER PROCESS!
        for (int i = 0; i < k; i++) {
            for (int j = 0; j < D; j++) {
                global_centroids[i * D + j] /= processes;  // Calculate average across all processes
            }
        }

        if (!rank) {
            for (int i = 0; i < k; i++) {
                printf("Epoch %d Centroid %d: x: %f y: %f z: %f\n", epoch, i, global_centroids[i * D + 0], global_centroids[i * D + 1], global_centroids[i * D + 2]);
            }
        }

        // Now, update the centroids with the synchronized global centroids
        for (int i = 0; i < k; i++) {
            centroid_array[i * D + 0] = global_centroids[i * D + 0];
            centroid_array[i * D + 1] = global_centroids[i * D + 1];
            centroid_array[i * D + 2] = global_centroids[i * D + 2];
        }
    }

    free(global_centroids);  // Clean up

    printf("Process %d finished running K-means\n", rank);

    MPI_Finalize();

    free(localpoints);
    free(centroids);
    free(coords);
    free(assignments);
    free(centroid_array);

    return 0;
}