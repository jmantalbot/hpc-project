# USU CS 5030/6030 Final Project

## Authors

- Brigham Campbell
- Josh Talbot
- Matthew Hill

## Genre Reveal Party

Our group chose project 2 -- Genre Reveal Party. This involves using a k-means algorithm to cluster Spotify songs based.

## Build & Run Instructions

1. Connect to NotchPeak on CHPC
  - required for GPU, kingspeak will not work (in our experience)
1. Load all necessary modules on CHPC
  - `module load gcc/11.2.0`
  - `module load cuda/12.5.0`
  - `module --latest load openmpi`
  - `module --latest load cmake`
  - optional modules:
    - `module load git-lfs`
1. Build
  - `bash build.sh`
    - installing boost can take a while, be patient!
1. Run the binaries
  - Serial: `./build/genre_reveal_party`
  - OMP: `TBD
  - MPI: `mpirun -n <number_of_processes> ./build/genre_reveal_party_mpi`
  - CUDA: `TBD`
  - CUDA w/ MPI: `TBD`

## Dataset

[CSV of 12 million spotify songs](https://www.kaggle.com/datasets/rodolfofigueroa/spotify-12m-songs)
