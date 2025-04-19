import os
import sys
import matplotlib.pyplot as plt
import subprocess
import time
import shutil
import csv

JOB_MAIL_TYPE = "FAIL,BEGIN,END",
JOB_MAIL_USER = "potatoman.mh@gmail.com" # Matthew Hill's email
DEFAULT_TIME = "0:15:00"
CPU_PARTITION = "notchpeak-gpu"
CPU_ACCOUNT = "notchpeak-gpu"
GPU_PARTITION = "notchpeak-gpu"
GPU_ACCOUNT = "notchpeak-gpu"

RESULTS_DIRECTORY = "scaling_study/results"
SERIAL_TIMING_FILE = os.path.join(RESULTS_DIRECTORY, "serial_timing.csv")
OMP_TIMING_FILE = os.path.join(RESULTS_DIRECTORY, "omp_timing.csv")

SCRATCH_DIR = os.getcwd()

THREAD_COUNT_HEADER = "thread_count"
PROCESSES_PER_NODE_HEADER = "processes_per_node"
TOTAL_PROCESSES_PER_NODE_HEADER = "total_processes"
EXECUTION_TIME_HEADER = "execution_time"


def queue_serial_job():
  """
  Demonstrate that the execution time of the serial algorithm 
  is consistent for different node and thread counts
  """
  print("queueing job for serial target...")
  command = [
    "sbatch",
    'scaling_study/slurm_run_serial.sh',
  ]
  subprocess.run(command,
  stdout=None,
  check=True,
  stderr=subprocess.STDOUT
  )

def queue_omp_job():
  print("queueing job for OMP target...")
  command = [
    "sbatch",
    'scaling_study/slurm_run_omp.sh',
  ]
  subprocess.run(command,
    stdout=None,
    check=True,
    stderr=subprocess.STDOUT
  )

def queue_mpi_jobs():
  print("queueing jobs for MPI target...")
  # 1, 2, 3, and 4 nodes
  for node_count in range(1, 3):
    command = [
      "sbatch",
      f"--nodes={node_count}",
      "scaling_study/slurm_run_mpi.sh",
    ]
    subprocess.run(command,
      stdout=None,
      check=True,
      stderr=subprocess.STDOUT
    )

def queue_cuda_jobs():
  pass

def queue_cuda_mpi_jobs():
  print("CUDA with MPI not yet implemented.")

def run_all_jobs():
  try:
    # queue jobs
    # queue_serial_job()
    # queue_omp_job()
    queue_mpi_jobs()
    # queue_cuda_jobs()
    # queue_cuda_mpi_jobs()
    # wait until all jobs are done
    while subprocess.run(["squeue", "-u", f"{os.getenv('USER')}", "--noheader"], capture_output=True, text=True).stdout.strip():
      print("waiting for jobs to finish, run watch 'squeue -u $USER' to see the job queue...")
      time.sleep(15)
  except KeyboardInterrupt as e:
    subprocess.run(["scancel", "-u", f"{os.getenv('USER')}"])
    raise e
  except subprocess.CalledProcessError as e:
    print(f"\nan sbatch command failed.\nError Code: {e.returncode}\noutput:\n{e.stderr}\n")
    subprocess.run(["scancel", "-u", f"{os.getenv('USER')}"])
    raise e

  print("All jobs done.")

def setup_plot(xlabel, ylabel, log_scale = False):
  plt.xlabel(xlabel)
  plt.ylabel(ylabel)
  if log_scale:
    plt.xscale('log')

def plot_serial_results():
  results = []
  with open(SERIAL_TIMING_FILE, "r", encoding="utf-8") as timing_file:
    csv_reader = csv.DictReader(timing_file)
    for row in csv_reader:
      results.append((int(row[THREAD_COUNT_HEADER]), float(row[EXECUTION_TIME_HEADER])))
  results.sort(key=lambda x: x[0])
  x, y = zip(*results)
  plt.plot(x, y, label="Serial")


def plot_omp_results():
  results = []
  with open(OMP_TIMING_FILE, "r", encoding="utf-8") as timing_file:
    csv_reader = csv.DictReader(timing_file)
    for row in csv_reader:
      results.append((int(row[THREAD_COUNT_HEADER]), float(row[EXECUTION_TIME_HEADER])))
  results.sort(key=lambda x: x[0])
  x, y = zip(*results)
  plt.plot(x, y, label="OMP")

def plot_mpi_results():
  for node_count in range(1, 5):
    filepath = os.path.join(RESULTS_DIRECTORY, f"mpi_timing_{node_count}.csv")
    if not os.path.exists(filepath):
      continue
    results = []
    with open(filepath, "r", encoding="utf-8") as timing_file:
      csv_reader = csv.DictReader(timing_file)
      for row in csv_reader:
        results.append(
          (
            int(row[PROCESSES_PER_NODE_HEADER]),
            float(row[EXECUTION_TIME_HEADER])
          )
        )
    results.sort(key=lambda x: x[0])
    x, y = zip(*results)
    plt.plot(x, y, label=f"MPI {node_count} Nodes")

def plot_cuda_results():
  pass

def plot_cuda_mpi_results():
  pass

def add_line_to_plot(x, y, label):
  # results.sort(key=lambda x: x[0])
  # x, y = zip(*results)
  plt.plot(x, y, label=label)

  
def plot_results():
  plt.clf()
  # Serial and OMP in one plot
  # setup_plot("Number of threads", "Time (s)", False)
  # plot_serial_results()
  # plot_omp_results()
  # save_plot(os.path.join(RESULTS_DIRECTORY, "serial_omp_results.png"))
  # plt.clf()

  # MPI Plot
  setup_plot("Number of Processes Per Node", "Time (s)", False)
  plot_mpi_results()
  save_plot(os.path.join(RESULTS_DIRECTORY, "mpi_results.png"))
  plt.clf()

  # # CUDA Plot
  # setup_plot("Grid Width", "Time (s)", False)
  # plot_cuda_results()
  # save_plot(os.path.join(RESULTS_DIRECTORY, "cuda_results.png"))
  # plt.clf()

  # # CUDA + MPI Plot
  # setup_plot("Unsure yet", "Time (s)", False)
  # plot_cuda_mpi_results()
  # save_plot(os.path.join(RESULTS_DIRECTORY, "cuda_mpi_results.png"))

def save_plot(filepath: str = "study_results.png"):
  plt.legend()
  plt.savefig(filepath)

def main():
  print("Starting scaling study.")
  os.makedirs(RESULTS_DIRECTORY, exist_ok=True)
  # run_all_jobs()
  plot_results()

if __name__ == "__main__":
  main()