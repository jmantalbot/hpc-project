import os
import sys
import matplotlib.pyplot as plt
import subprocess
import time

INPUT_DATA_FILE = "data/spotify.csv"
OUTPUT_DATA_FILE = "data/spotify_clusters.csv"
PROCESS_COUNTS = [1, 4, 16, 64, 256, 1024, 4096, 16384, 65536]
REPETITIONS = 20

SERIAL_OUTPUT_DATA_FILE = "data/serial.csv"
OMP_OUTPUT_DATA_FILE = "data/omp.csv"
MPI_OUTPUT_DATA_FILE = "data/mpi.csv"
CUDA_OUTPUT_DATA_FILE = "data/cuda.csv"
CUDA_MPI_OUTPUT_DATA_FILES = "data/cuda_mpi.csv"

SERIAL_EXECUTABLE = "./build/genre_reveal_party"
OMP_EXECUTABLE = "./build/genre_reveal_party_omp"
MPI_EXECUTABLE = "./build/genre_reveal_party_mpi"
CUDA_EXECUTABLE = "./build/genre_reveal_party_cuda"
CUDA_MPI_EXECUTABLE = "./build/genre_reveal_party_cuda_mpi"

def run_serial_target():
  print("running serial target...")
  results = dict.fromkeys(PROCESS_COUNTS)
  start_time = 0
  end_time = 0
  for process_count in PROCESS_COUNTS:
    print(f"{process_count} processes....")
    elapsed_time = 0
    for _ in range(REPETITIONS):
      start_time = time.perf_counter()
      subprocess.run(SERIAL_EXECUTABLE, stdout=subprocess.DEVNULL)
      end_time = time.perf_counter()
      elapsed_time += end_time - start_time
    results[process_count] = elapsed_time / REPETITIONS
  os.rename(OUTPUT_DATA_FILE, SERIAL_OUTPUT_DATA_FILE)
  print("Done.")
  return results

def setup_plot():
  plt.xlabel("Number of Processes / Threads")
  plt.ylabel("Average Execution Time")
  plt.xscale('log')
  

def plot_results(results: dict[int, float], label: str):
  sorted_results = sorted(results.items())
  x, y = zip(*sorted_results)
  plt.plot(x, y, label=label)

def main():
  setup_plot()
  serial_results = run_serial_target()
  plot_results(serial_results, label="Serial")
  plt.legend()
  plt.savefig("study_results.png")



if __name__ == "__main__":
  main()