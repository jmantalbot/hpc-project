#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "cluster.h"

#define BUF_SIZE 262144

const int DIMENSIONS = 13;
const int INDICES[] = {8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 21};

struct point {
    int line_number;
    float dim[13];
};

struct point *localpoints;
struct point *centroids;
int num_points;
int k;
int lines = -1; // Don't count the header
FILE *csvfp;
char buf[BUF_SIZE];

int cmp(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}

void setup_and_scatter_csv(int argc, char *argv[], int rank, int processes) {
    int lines_per_process;
    csvfp = fopen(argv[1], "r");
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
        if (col < 21) {
            fprintf(stderr, "Process %d: Incorrect number of columns on line %d\n", rank, current_line);
        }

        if (current_line >= startline && current_line < displs) {
            struct point p;
            p.line_number = current_line + 1;
            for (int i = 0; i < DIMENSIONS; i++) {
                p.dim[i] = strtof(tokens[INDICES[i]], NULL);
            }
            localpoints[local_index++] = p;
            if (current_line == 2) {
                for (int i = 0; i < DIMENSIONS; i++) {
                    printf("%f, ", p.dim[i]);
                }
                printf("\n");
            }
        }

        if (centroid_index < k && centroid_indices[centroid_index] == current_line) { 
            struct point centroid;
            centroid.line_number = current_line + 1;
            for (int i = 0; i < DIMENSIONS; i++) {
                centroid.dim[i] = strtof(tokens[INDICES[i]], NULL);
            }
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
    float* coords = malloc(num_points * DIMENSIONS * sizeof(float));
    int* assignments = malloc(num_points * sizeof(int));
    float* centroid_array = malloc(k * DIMENSIONS * sizeof(float));

    for (int i = 0; i < num_points; i++) {
        for (int j = 0; j < DIMENSIONS; j++) {
            coords[i * DIMENSIONS + j] = localpoints[i].dim[j];
        }
    }

    // Initialize centroids[] based on the global centroids
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < DIMENSIONS; j++) {
            centroid_array[i * DIMENSIONS + j] = centroids[i].dim[j];
        }
    }

    // Synchronize centroids across all processes using MPI_Allreduce
    // We use MPI_Allreduce to compute the global centroid for each cluster (mean of all centroids)
    float* global_centroids = malloc(k * DIMENSIONS * sizeof(float));

    for (int epoch = 0; epoch < maxEpochs; epoch++) {

        // Run the CUDA-based K-means algorithm
        run_kmeans_gpu(coords, assignments, centroid_array, num_points, k, 1, DIMENSIONS); // 1 epoch at a time

        MPI_Allreduce(centroid_array, global_centroids, k * DIMENSIONS, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);

        // Now we divide by the number of processes to get the average centroid
        // TODO: THIS SHOULD BE A WEIGHTED AVERAGE USING THE NUMBER OF POINTS PER PROCESS!
        for (int i = 0; i < k; i++) {
            for (int j = 0; j < DIMENSIONS; j++) {
                global_centroids[i * DIMENSIONS + j] /= processes;  // Calculate average across all processes
            }
        }

        if (!rank) {
            printf("\n");
            for (int i = 0; i < k; i++) {
                printf("Epoch %d Centroid %d:\n", epoch, i);
                for (int j = 0; j < DIMENSIONS; j++) {
                    printf("%f, ", global_centroids[i * DIMENSIONS + j]);
                }
                printf("\n");
            }
        }

        // Now, update the centroids with the synchronized global centroids
        for (int i = 0; i < k; i++) {
            for (int j = 0; j < DIMENSIONS; j++) {
                centroid_array[i * DIMENSIONS + j] = centroids[i].dim[j];
            }
        }
    }

    free(global_centroids);  // Clean up

    printf("Process %d finished running K-means\n", rank);
    
    int* all_assignments = NULL;
    int* recvcounts = NULL;
    int* displs = NULL;

    if (rank == 0) {
        all_assignments = malloc(lines * sizeof(int));
        recvcounts = malloc(processes * sizeof(int));
        displs = malloc(processes * sizeof(int));
    }

    // Gather how many points each process has
    int local_count = num_points;
    int* all_counts = NULL;
    if (rank == 0) {
        all_counts = malloc(processes * sizeof(int));
    }

    MPI_Gather(&local_count, 1, MPI_INT, all_counts, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        displs[0] = 0;
        recvcounts[0] = all_counts[0];
        for (int i = 1; i < processes; i++) {
            recvcounts[i] = all_counts[i];
            displs[i] = displs[i - 1] + recvcounts[i - 1];
        }
    }

    MPI_Gatherv(assignments, num_points, MPI_INT,
                all_assignments, recvcounts, displs, MPI_INT,
                0, MPI_COMM_WORLD);

    MPI_Finalize();

    if (rank == 0) {
        int point_id = 0;
        FILE *outputfp = fopen("./data/spotify_clusters.csv", "w");
        csvfp = fopen(argv[1], "r");
        fgets(buf, BUF_SIZE, csvfp);
        buf[strcspn(buf, "\r\n")] = 0;
        fprintf(outputfp, "%s,cluster\n", buf);
        while (fgets(buf, BUF_SIZE, csvfp)) {
            buf[strcspn(buf, "\r\n")] = 0;
            fprintf(outputfp, "%s,%d\n", buf, all_assignments[point_id++]);
        }

        fclose(outputfp);

        free(all_assignments);
        free(recvcounts);
        free(displs);
        free(all_counts);
    }

    free(localpoints);
    free(centroids);
    free(coords);
    free(assignments);
    free(centroid_array);

    return 0;
}