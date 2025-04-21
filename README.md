# USU CS 5030/6030 Final Project

## Authors

- Brigham Campbell
- Josh Talbot
- Matthew Hill

## Genre Reveal Party

Our group chose project 2 -- Genre Reveal Party. This involves using a k-means algorithm to cluster Spotify songs based.

## Build Instructions

1. Connect to NotchPeak on CHPC
  - required for GPU, kingspeak will not work (in our experience)
1. Load all necessary modules on CHPC
  - `module load python/3.10.3 gcc/11.2.0 cuda/12.5.0 openmpi/5.0.3 cmake/3.26.0`
  - optional modules:
    - `module load git-lfs`
1. Build
  - `bash build.sh`
    - installing boost can take a while, be patient!
  - Binaries will be at `build/`

## Run Instructions

There are some options:

1. Run any individual target
  - `./build/genre_reveal_party data/spotify.csv`
  - `./build/genre_reveal_party_omp data/spotify.csv`
  - `./build/genre_reveal_party_cuda data/spotify.csv`
  - `./build/genre_reveal_party_mpi data/spotify.csv`
  - `./build/genre_reveal_party_cuda_mpi data/spotify.csv`
1. Run validation script, which runs all the resulting binaries & checks against the serial results. It does this on a subset of the full spotify data, only 500 tracks, to make it run much faster.
  - `bash validation.sh`
1. Run the scaling study
  - `python scaling_study/scaling_study.py`
  - Run `watch 'squeue -u $USER'` in another terminal to monitor the progress
  - Results will be in the `scaling_study/results/` directory
  - Logs will be in the `scaling_study/logs/` directory

You can visualize the results after running any of the scripts by running `python scaling_study/visualization.py <data_file.csv>` and give the keys for the axes. The columns can be easily viewed in `data/spotify_short.csv`. The results are saved to `plt.png` in your working directory (likely the repo root).

## Dataset

[CSV of 12 million spotify songs](https://www.kaggle.com/datasets/rodolfofigueroa/spotify-12m-songs)

## Approach Descriptions & Analysis

### Serial Implementation

Matthew Hill & Josua Talbot

- We made a point class containing the location of each point, as well as which cluster that point is assigned to, and the distance to the closest cluster
- Main reads in the csv file containing the spotify data, performs the k-means-clustering algorithm, then writes it to a new file.
- The clustering algorithm is as follow:
  - We have k centroids. The location of each is randomized. We chose a set seed so that the output would be predictable and consistent.
  - while more than one point changed:
    - for each point, find the nearest centroid, and assign the point to that cluster.
    - move the centroid to the average location of the points assigned to it.
      

### OMP Implementation (Shared memory CPU)

Joshua Talbot

- This algorithm is extremely similar to the serial approach, using OpenMP in places that parallelization can be easily implemented.
- Using this method we parallelized the reading and the writing of the data to files in main
- Also implemented was Parallelization for calculating the mimimum distances for each point and moving the centroids. 

CUDA Implementation (Joshua Talbot)
- CUDA parallelization was implemented in our cluster.cu file. We used kernal functions for computing the distances, the computation of summations, and updating the centroids.
- The CUDA implementation is pretty different from the OMP and Serial Implementations, but the results are much the same.
  - We set up some random centroids
  - We compute the distances and update the centroids
  - If no points have changes, we break out, just like OMP and the Serial

### MPI Implementation (Distributed memory CPU)

Matthew Hill

- By scattering the points among the processes, each process is responsible for N / P of the points, where N = number of processes and P = number of points.
- Each process computes the sum of the coordinates of its points. They are reduced into the root process which computes the average position for each cluster then moves the centroid points to the computed average positions.
- At the beginning of each epoch, the centroids are broadcast from the root process to all other processes.
- After the centroids converge or the max epoch is reached, the points are gathered into the root process to be written to the output file.
- Boost is used instead of plain MPI.
  - Boost makes scattering, gathering, and broadcasting vectors of Points, which themselves have a vector attributes, much simpler with it's MPI and Serialization libraries.
- This implementation can use a very large amount of memory. If it is unable to be run on CHPC with the provided scripts, configure the --mem-per-cpu option or try running on a different partition.

### CUDA Implementation (Shared memory GPU)

- CUDA parallelization was implemented in our cluster.cu file. We used kernal functions for computing the distances, the computation of summations, and updating the centroids.
- The CUDA implementation is pretty different from the OMP and Serial Implementations, but the results are much the same.
  - We set up some random centroids
  - We compute the distances and update the centroids
  - If no points have changes, we break out, just like OMP and the Serial

### CUDA + MPI Implementation (Distributed memory GPU)

Brigham Campbell

- 

## Scaling Study

