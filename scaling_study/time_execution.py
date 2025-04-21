import argparse
import os
import time
import subprocess
import csv

REPETITIONS = 1

SERIAL_EXECUTABLE = "./build/genre_reveal_party"
OMP_EXECUTABLE = "./build/genre_reveal_party_omp"
MPI_EXECUTABLE = "./build/genre_reveal_party_mpi"
CUDA_EXECUTABLE = "./build/genre_reveal_party_cuda"
CUDA_MPI_EXECUTABLE = "./build/genre_reveal_party_mpi_cuda"

THREAD_COUNT_HEADER = "thread_count"
PROCESSES_PER_NODE_HEADER = "processes_per_node"
TOTAL_PROCESSES_PER_NODE_HEADER = "total_processes"
BLOCK_SIZE_HEADER = "block_size"
EXECUTION_TIME_HEADER = "execution_time"

DATA_FILE = "data/spotify.csv"


OMP_THREAD_COUNTS = [1, 16, 32, 64, 1024]
MPI_PROCESSES_PER_NODE = [1, 8, 16, 32]
BLOCK_SIZES = [1, 64, 128, 1023]
MPI_CUDA_PROCESSES_PER_NODE = [1, 2, 3, 4]

def time_serial(
    timing_output_file: str,
):
  print("timing serial...")
  results = []
  for thread_count in OMP_THREAD_COUNTS:
    print(f"thread count: {thread_count}")
    try:
      # os.chmod(timing_output_file, 0o777)
      elapsed_time = 0
      for _ in range(REPETITIONS):
        start_time = time.perf_counter()
        subprocess.run(
          [SERIAL_EXECUTABLE, DATA_FILE],
          stdout=None,
          check=True,
          stderr=subprocess.STDOUT,
        )
        end_time = time.perf_counter()
        elapsed_time += end_time - start_time
      results.append((thread_count, elapsed_time / REPETITIONS))
    except subprocess.CalledProcessError as e:
      print(f"\nSERIAL: Command failed for {thread_count} threads.\nError Code: {e.returncode}\noutput:\n{e.stderr}\n")
    except FileNotFoundError as e:
      print(f"SERIAL: Command or File not found: {e.filename}")
  with open(timing_output_file, "w+") as csv_file:
    csv_writer = csv.DictWriter(csv_file, fieldnames=[THREAD_COUNT_HEADER, EXECUTION_TIME_HEADER])
    csv_writer.writeheader()
    for result in results:
      csv_writer.writerow({
        THREAD_COUNT_HEADER: f"{result[0]}",
        EXECUTION_TIME_HEADER: f"{result[1]}",
      })
  return

def time_omp(
    timing_output_file: str,
):
  print("timing OMP...")
  results = []
  for thread_count in OMP_THREAD_COUNTS:
    print(f"thread count: {thread_count}")
    try:
      # os.chmod(timing_output_file, 0o777)
      elapsed_time = 0
      for _ in range(REPETITIONS):
        start_time = time.perf_counter()
        subprocess.run(
          [OMP_EXECUTABLE, DATA_FILE, str(thread_count)],
          stdout=None,
          check=True,
          stderr=subprocess.STDOUT,
        )
        end_time = time.perf_counter()
        elapsed_time += end_time - start_time
      results.append((thread_count, elapsed_time / REPETITIONS))
    except subprocess.CalledProcessError as e:
      print(f"\OMP: Command failed for {thread_count} threads.\nError Code: {e.returncode}\noutput:\n{e.stderr}\n")
    except FileNotFoundError as e:
      print(f"OMP: Command or File not found: {e.filename}")
  with open(timing_output_file, "w+") as csv_file:
    csv_writer = csv.DictWriter(csv_file, fieldnames=[THREAD_COUNT_HEADER, EXECUTION_TIME_HEADER])
    csv_writer.writeheader()
    for result in results:
      csv_writer.writerow({
        THREAD_COUNT_HEADER: f"{result[0]}",
        EXECUTION_TIME_HEADER: f"{result[1]}",
      })
  return

def time_mpi(
  timing_output_file: str,
  node_count: int,
):
  print(f"timing MPI with {node_count} nodes...")
  results = []
  for processes_per_node in MPI_PROCESSES_PER_NODE:
    total_process_count = processes_per_node * node_count
    print(f"total process count: {total_process_count}")
    try:
      # os.chmod(timing_output_file, 0o777)
      elapsed_time = 0
      for _ in range(REPETITIONS):
        start_time = time.perf_counter()
        subprocess.run(
          [
            "mpirun",
            "-n",
            f"{total_process_count}",
            "--display-map",
            "--map-by",
            ":OVERSUBSCRIBE",
            MPI_EXECUTABLE,
            DATA_FILE
          ],
          stdout=None,
          check=True,
          stderr=subprocess.STDOUT,
        )
        end_time = time.perf_counter()
        elapsed_time += end_time - start_time
      results.append((processes_per_node, elapsed_time / REPETITIONS))
    except subprocess.CalledProcessError as e:
      print(f"\MPI: Command failed for {total_process_count} processes across {node_count} nodes.\nError Code: {e.returncode}\noutput:\n{e.stderr}\n")
    except FileNotFoundError as e:
      print(f"MPI: Command or File not found: {e.filename}")
  with open(timing_output_file, "w+") as csv_file:
    csv_writer = csv.DictWriter(csv_file, fieldnames=[PROCESSES_PER_NODE_HEADER, TOTAL_PROCESSES_PER_NODE_HEADER, EXECUTION_TIME_HEADER])
    csv_writer.writeheader()
    for result in results:
      csv_writer.writerow({
        PROCESSES_PER_NODE_HEADER: f"{result[0]}",
        EXECUTION_TIME_HEADER: f"{result[1]}",
        TOTAL_PROCESSES_PER_NODE_HEADER: f"{result[0] * node_count}"
      })

def time_cuda(
  timing_output_file
):
  print("timing CUDA...")
  results = []
  for block_size in BLOCK_SIZES:
    print(f"thread count: {block_size}")
    try:
      # os.chmod(timing_output_file, 0o777)
      elapsed_time = 0
      for _ in range(REPETITIONS):
        start_time = time.perf_counter()
        subprocess.run(
          [CUDA_EXECUTABLE, "data/spotify_medium.csv", str(block_size)],
          stdout=None,
          check=True,
          stderr=subprocess.STDOUT,
        )
        end_time = time.perf_counter()
        elapsed_time += end_time - start_time
      results.append((block_size, elapsed_time / REPETITIONS))
    except subprocess.CalledProcessError as e:
      print(f"\CUDA: Command failed for {block_size} block size.\nError Code: {e.returncode}\noutput:\n{e.stderr}\n")
    except FileNotFoundError as e:
      print(f"CUDA: Command or File not found: {e.filename}")
  with open(timing_output_file, "w+") as csv_file:
    csv_writer = csv.DictWriter(csv_file, fieldnames=[BLOCK_SIZE_HEADER, EXECUTION_TIME_HEADER])
    csv_writer.writeheader()
    for result in results:
      csv_writer.writerow({
        BLOCK_SIZE_HEADER: f"{result[0]}",
        EXECUTION_TIME_HEADER: f"{result[1]}",
      })


def time_cuda_mpi(
  timing_output_file: str,
  node_count: int,
):
  print(f"timing MPI with {node_count} nodes...")
  results = []
  for processes_per_node in MPI_CUDA_PROCESSES_PER_NODE:
    total_process_count = processes_per_node * node_count
    print(f"total process count: {total_process_count}")
    try:
      # os.chmod(timing_output_file, 0o777)
      elapsed_time = 0
      for _ in range(REPETITIONS):
        start_time = time.perf_counter()
        subprocess.run(
          [
            "mpirun",
            "-n",
            f"{total_process_count}",
            "--display-map",
            "--map-by",
            ":OVERSUBSCRIBE",
            CUDA_MPI_EXECUTABLE,
            DATA_FILE
          ],
          stdout=None,
          check=True,
          stderr=subprocess.STDOUT,
        )
        end_time = time.perf_counter()
        elapsed_time += end_time - start_time
      results.append((processes_per_node, elapsed_time / REPETITIONS))
    except subprocess.CalledProcessError as e:
      print(f"\MPI: Command failed for {total_process_count} processes across {node_count} nodes.\nError Code: {e.returncode}\noutput:\n{e.stderr}\n")
    except FileNotFoundError as e:
      print(f"MPI: Command or File not found: {e.filename}")
  with open(timing_output_file, "w+") as csv_file:
    csv_writer = csv.DictWriter(csv_file, fieldnames=[PROCESSES_PER_NODE_HEADER, TOTAL_PROCESSES_PER_NODE_HEADER, EXECUTION_TIME_HEADER])
    csv_writer.writeheader()
    for result in results:
      csv_writer.writerow({
        PROCESSES_PER_NODE_HEADER: f"{result[0]}",
        EXECUTION_TIME_HEADER: f"{result[1]}",
        TOTAL_PROCESSES_PER_NODE_HEADER: f"{result[0] * node_count}"
      })
  

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
  parser.add_argument("--output", required=True)
  # mpi
  # node-count * ntasks-per-node = process count
  parser.add_argument("--nodes")
  # cuda
  # blocks per grid by each dimension
  parser.add_argument("--grid_x")
  parser.add_argument("--grid_y")
  parser.add_argument("--grid_z")
  # threads per block by each dimension
  parser.add_argument("--block_x")
  parser.add_argument("--block_y")
  parser.add_argument("--block_z")
  # blocks in grid (x * y * z) * threads per block (x * y * z) = total thread count

  args = parser.parse_args()
  return args

def main():
  args = parse_args()
  if args.target == "serial":
    time_serial(args.output)
  elif args.target == "omp":
    time_omp(args.output)
  elif args.target == "mpi":
    time_mpi(args.output, int(args.nodes))
  elif args.target == "cuda":
    time_cuda(args.output)
  elif args.target == "cuda_mpi":
    time_cuda_mpi(args.output, int(args.nodes))

if __name__ == main():
  main()