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
  - `./build/genre_reveal_party`
  - `./build/genre_reveal_party_omp`
  - `./build/genre_reveal_party_cuda`
  - `./build/genre_reveal_party_mpi`
  - `./build/genre_reveal_party_cuda_mpi`
1. Run validation script, which runs all the resulting binaries & checks against the serial results
  - `bash validation.sh`
1. Run the scaling study
  - `python scaling_study/scaling_study.py`
  - Run `watch 'squeue -u $USER'` in another terminal to monitor the progress
  - Results will be in the `scaling_study/results/` directory
  - Logs will be in the `scaling_study/logs/` directory

You can visualize the results after running any of the scripts by running `python scaling_study/visualization.py <data_file.csv>` and give the keys for the axes. The columns can be easily viewed in `data/spotify_short.csv`. The results are saved to `plt.png` in your working directory (likely the repo root).

## Dataset

[CSV of 12 million spotify songs](https://www.kaggle.com/datasets/rodolfofigueroa/spotify-12m-songs)
