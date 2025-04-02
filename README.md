# USU CS 5030/6030 Final Project

## Authors

- Brigham Campbell
- Josh Talbot
- Matthew Hill

## Genre Reveal Party

Our group chose project 2 -- Genre Reveal Party. This involves using a k-means algorithm to cluster Spotify songs based.

## Build & Run Instructions

1. Build
  - `bash build.sh`
1. Run the binary
  - `./build/genre_reveal_party`

## MPI approach

2 stages: points and centroids

1. Scatterv/Gatherv/Reduce BY POINTS
  1. Expecting typically Number of processes <<< number of data points (songs) (N <<< P)
  1. Scatter points
  1. Broadcast centroids
  1. reassign (local) points to clusters
    1. determine if no changes are made to assignments (reduce? allreduce?)
  1. Compute sum of coordinates of (local) points for each cluster & count of points in each cluster
  1. Gather points
  1. Reduce average coordinate for each cluster
1. Update clusters (serial)
  1. Expecting typically Number of processes >>> number of clusters (N >>> K)
  1. update clusters in serial or w/ OMP. Reduce gather/scatter overhead.
  1. parallel option (Scatterv/Gatherv)
    1. Scatter centroids and averages
    1. Move centroid to average
    1. Gather centroids

## Dataset

[CSV of 12 million spotify songs](https://www.kaggle.com/datasets/rodolfofigueroa/spotify-12m-songs)
