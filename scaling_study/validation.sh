#!/bin/bash

mkdir -p data/out

./build/genre_reveal_party data/spotify_short.csv
mv data/spotify_clusters.csv data/out/serial.csv

./build/genre_reveal_party_omp data/spotify_short.csv 13
mv data/spotify_clusters.csv data/out/omp.csv

mpirun -n 17 ./build/genre_reveal_party_mpi data/spotify_short.csv
mv data/spotify_clusters.csv data/out/mpi_17.csv

./build/genre_reveal_party_cuda data/spotify_short.csv
mv data/spotify_clusters.csv data/out/cuda.csv

if [[ -z "$(diff data/out/serial.csv data/out/omp.csv)" ]]; then
  echo "serial and omp outputs are the same 🎉"
else
  echo "ERROR: serial and omp outputs are different. Run `diff data/out/serial.csv data/out/omp.csv` to see differences"
fi

if [[ -z "$(diff data/out/serial.csv data/out/mpi_17.csv)" ]]; then
  echo "serial and mpi outputs are the same 🎉"
else
  echo "ERROR: serial and mpi outputs are different. Run `diff data/out/serial.csv data/out/mpi_17.csv` to see differences"
fi

if [[ -z "$(diff data/out/serial.csv data/out/cuda.csv)" ]]; then
  echo "serial and cuda outputs are the same 🎉"
else
  echo "ERROR: serial and cuda outputs are different. Run `diff data/out/serial.csv data/out/cuda.csv` to see differences"
fi