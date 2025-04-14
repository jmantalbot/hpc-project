#!/bin/bash

./build/genre_reveal_party
mv data/spotify_clusters.csv data/serial.csv

./build/genre_reveal_party_omp 27
mv data/spotify_clusters.csv data/omp.csv

mpirun -n 17 ./build/genre_reveal_party_mpi
mv data/spotify_clusters.csv data/mpi_17.csv

./build/genre_reveal_party_cuda
mv data/spotify_clusters.csv data/cuda.csv

if [[ -z "$(diff data/serial.csv data/omp.csv)" ]]; then
  echo "serial and omp outputs are the same 🎉"
else
  echo "ERROR: serial and omp outputs are different. Run `diff data/serial.csv data/omp.csv` to see differences"
fi

if [[ -z "$(diff data/serial.csv data/mpi_17.csv)" ]]; then
  echo "serial and mpi outputs are the same 🎉"
else
  echo "ERROR: serial and mpi outputs are different. Run `diff data/serial.csv data/mpi_17.csv` to see differences"
fi

if [[ -z "$(diff data/serial.csv data/cuda.csv)" ]]; then
  echo "serial and cuda outputs are the same 🎉"
else
  echo "ERROR: serial and cuda outputs are different. Run `diff data/serial.csv data/cuda.csv` to see differences"
fi