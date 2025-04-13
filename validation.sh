#!/bin/bash

./build/genre_reveal_party
mv data/spotify_clusters.csv data/serial.csv

mpirun -n 17 ./build/genre_reveal_party_mpi
mv data/spotify_clusters.csv data/mpi_17.csv

diff data/serial.csv data/mpi_17.csv