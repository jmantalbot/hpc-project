import os
import sys
import matplotlib.pyplot as plt
import subprocess
import time

INPUT_DATA_FILE = "data/spotify.csv"
OUTPUT_DATA_FILE = "data/spotify_clusters.csv"
PROCESS_COUNTS = [1, 4, 16, 32, 64, 128, 256]#, 1024, 4096, 16384, 65536]
NODE_COUNTS = [2, 3, 4]
REPETITIONS = 1 #TODO: change to 20

SERIAL_OUTPUT_DATA_FILE = "data/out/serial.csv"
OMP_OUTPUT_DATA_FILE = "data/out/omp.csv"
MPI_OUTPUT_DATA_FILE = "data/out/mpi.csv"
CUDA_OUTPUT_DATA_FILE = "data/out/cuda.csv"
CUDA_MPI_OUTPUT_DATA_FILES = "data/out/cuda_mpi.csv"

SERIAL_EXECUTABLE = "./build/genre_reveal_party"
OMP_EXECUTABLE = "./build/genre_reveal_party_omp"
MPI_EXECUTABLE = "./build/genre_reveal_party_mpi"
CUDA_EXECUTABLE = "./build/genre_reveal_party_cuda"
CUDA_MPI_EXECUTABLE = "./build/genre_reveal_party_cuda_mpi"

def run_target(command, process_count: int, stdout = subprocess.DEVNULL) -> float:
  print(f"{process_count} processes...")
  elapsed_time = 0
  try:
    for _ in range(REPETITIONS):
      start_time = time.perf_counter()
      subprocess.run(
        command,
        stdout=stdout,
        check=True,
        stderr=subprocess.STDOUT
      )
      end_time = time.perf_counter()
      elapsed_time += end_time - start_time
    return elapsed_time / REPETITIONS
  except subprocess.CalledProcessError as e:
    print(f"\nCommand {command} failed for {process_count} processes.\nError Code: {e.returncode}\noutput:\n{e.stderr}\n")
    return None
  except FileNotFoundError as e:
    print(f"Command not found: {command}")
    return None
  

def run_and_plot_serial():
  """
  Run and time the serial target for the genre reveal party
  """
  print("running serial target...")
  results: dict[int, float] = dict.fromkeys(PROCESS_COUNTS)
  for process_count in PROCESS_COUNTS:
    results[process_count] = run_target([SERIAL_EXECUTABLE], process_count=process_count)
  os.rename(OUTPUT_DATA_FILE, SERIAL_OUTPUT_DATA_FILE)
  print("Done.")
  plot_results(results, label="Serial")

def run_and_plot_omp():
  """
  Run and time the OMP target for the genre reveal party
  """
  print("running OMP target...")
  results: dict[int, float] = dict.fromkeys(PROCESS_COUNTS)
  for process_count in PROCESS_COUNTS:
    results[process_count] = run_target(
      [
        OMP_EXECUTABLE,
        str(process_count),
      ], 
      process_count=process_count
    )
  os.rename(OUTPUT_DATA_FILE, OMP_OUTPUT_DATA_FILE)
  print("Done.")
  plot_results(results, label="OMP")

def run_and_plot_mpi():
  """
  Run and time the MPI target for the genre reveal party
  """
  print("running MPI target...")
  results: dict[int, float] = dict.fromkeys(PROCESS_COUNTS)
  for process_count in PROCESS_COUNTS:
    results[process_count] = run_target(
      # [
      #   "mpirun",
      #   "-n",
      #   str(process_count),
      #   MPI_EXECUTABLE,
      # ], 
      [
        "mpirun", "-n", f"{process_count}", "--display-map", "--map-by", ":OVERSUBSCRIBE", "./build/genre_reveal_party_mpi"
      ],
      # [
      #   "mpirun", "-n", f"{process_count}",  "-map-by socket", "-bind-to socket", "--display-map", "./build/genre_reveal_party_mpi"
        
      # ],
      process_count=process_count,
      stdout = None
    )
  os.rename(OUTPUT_DATA_FILE, MPI_OUTPUT_DATA_FILE)
  print("Done.")
  plot_results(results, label="MPI")

def run_and_plot_cuda():
  """
  Run and time the CUDA target for the genre reveal party
  """
  print("running CUDA target...")
  results: dict[int, float] = dict.fromkeys(PROCESS_COUNTS)
  for process_count in PROCESS_COUNTS:
    results[process_count] = run_target(
      [CUDA_EXECUTABLE], 
      process_count=process_count
    )
  os.rename(OUTPUT_DATA_FILE, CUDA_OUTPUT_DATA_FILE)
  print("Done.")
  plot_results(results, label="CUDA")

def run_and_plot_cuda_mpi():
  print("CUDA with MPI not yet implemented.")

def setup_plot():
  plt.xlabel("Number of Processes / Threads")
  plt.ylabel("Average Execution Time (s)")
  plt.xscale('log')
  

def plot_results(results: dict[int, float], label: str):
  sorted_results = sorted(results.items())
  x, y = zip(*sorted_results)
  plt.plot(x, y, label=label)


def save_plot(filepath: str = "study_results.png"):
  plt.legend()
  plt.savefig("study_results.png")


def main():
  setup_plot()
  run_and_plot_serial()
  run_and_plot_omp()
  run_and_plot_mpi()
  run_and_plot_cuda()
  run_and_plot_cuda_mpi()
  save_plot()


if __name__ == "__main__":
  main()