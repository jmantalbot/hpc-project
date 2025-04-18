import argparse
import os
import time
import subprocess
import csv

REPETITIONS = 1 # TODO: change to a higher number

SERIAL_EXECUTABLE = "./build/genre_reveal_party"
OMP_EXECUTABLE = "./build/genre_reveal_party_omp"
MPI_EXECUTABLE = "./build/genre_reveal_party_mpi"
CUDA_EXECUTABLE = "./build/genre_reveal_party_cuda"
CUDA_MPI_EXECUTABLE = "./build/genre_reveal_party_cuda_mpi"

THREAD_COUNT_HEADER = "thread_count"
EXECUTION_TIME_HEADER = "execution_time"

def time_serial(
    timing_output_file: str,
    thread_count: int,
):
  try:
    elapsed_time = 0
    for _ in range(REPETITIONS):
      start_time = time.perf_counter()
      subprocess.run(
        [SERIAL_EXECUTABLE],
        stdout=None,
        check=True,
        stderr=subprocess.STDOUT,
      )
      end_time = time.perf_counter()
      elapsed_time += end_time - start_time
    
    with open(timing_output_file, "a", encoding="utf-8") as csv_file:
      csv_writer = csv.DictWriter(csv_file)
      csv_writer.writerow({
        THREAD_COUNT_HEADER: f"{thread_count}",
        EXECUTION_TIME_HEADER: f"{elapsed_time / REPETITIONS}",
      })
    return
  except subprocess.CalledProcessError as e:
    print(f"\nSERIAL: Command failed for {thread_count} threads.\nError Code: {e.returncode}\noutput:\n{e.stderr}\n")
    return
  except FileNotFoundError as e:
    print(f"SERIAL: Command or File not found: {e.stderr}")
    return


def parse_args():
  parser = argparse.ArgumentParser()

  parser.add_argument(
    "--target",
    choices=[
      "serial",
      "omp",
      "mpi",
      "cuda",
      "cuda_mpi",
    ],
    required=True
  )
  parser.add_argument("--timing-output-file", required=True)
  # serial & omp
  parser.add_argument("--thread-count")
  # mpi
  parser.add_argument("--node-count")
  parser.add_argument("--ntasks-per-node")
  # node-count * ntasks-per-node = process count
  # cuda
  # blocks per grid by each dimension
  parser.add_argument("--grid-x")
  parser.add_argument("--grid-y")
  parser.add_argument("--grid-z")
  # threads per block by each dimension
  parser.add_argument("--block-x")
  parser.add_argument("--block-y")
  parser.add_argument("--block-z")
  # blocks in grid (x * y * z) * threads per block (x * y * z) = total thread count

  args = parser.parse_args()
  return args

def main():
  args = parse_args()
  print("yo")
  if args.target == "serial":
    print("ey")
    time_serial(
      args.timing-output-file,
      args.thread-count
    )
  elif args.target == "omp":
    pass
  elif args.target == "mpi":
    pass
  elif args.target == "cuda":
    pass
  elif args.target == "cuda_mpi":
    print("CUDA + MPI not yet implemented")

if __name__ == main():
  main()