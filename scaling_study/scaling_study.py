import os
import sys
import matplotlib.pyplot as plt
import subprocess
import time
import shutil
import csv

REPETITIONS = 200
JOB_MAIL_TYPE = "FAIL,BEGIN,END",
JOB_MAIL_USER = "potatoman.mh@gmail.com" # Matthew Hill's email
DEFAULT_TIME = "0:15:00"
CPU_PARTITION = "notchpeak-gpu"
CPU_ACCOUNT = "notchpeak-gpu"
GPU_PARTITION = "notchpeak-gpu"
GPU_ACCOUNT = "notchpeak-gpu"

SERIAL_TIMING_FILE = "serial_timing.csv"

SCRATCH_DIR = f"/scratch/local/{os.getenv('USER')}/project"

THREAD_COUNT_HEADER = "thread_count"
EXECUTION_TIME_HEADER = "execution_time"

def recursive_chmod(path, mode):
    for root, dirs, files in os.walk(path):
        for d in dirs:
            os.chmod(os.path.join(root, d), mode)
        for f in files:
            os.chmod(os.path.join(root, f), mode)

def run_and_plot_serial():
  """
  Demonstrate that the execution time of the serial algorithm 
  is consistent for different node and thread counts
  """
  print("queueing job for seial target...")
  # clear timing file, write headers
  print(os.getcwd())
  with open(SERIAL_TIMING_FILE, "w+") as csv_file:
    csv_writer = csv.writer(csv_file)
    csv_writer.writerow(
      [
        THREAD_COUNT_HEADER,
        EXECUTION_TIME_HEADER
      ]
    )
  os.chmod(SERIAL_TIMING_FILE, 0o777)
  THREAD_COUNTS = [1, 16, 32, 48, 64]
  try:
    for thread_count in THREAD_COUNTS:
      command = [
        "sbatch",
        f"--time={DEFAULT_TIME}",
        f"--partition={CPU_PARTITION}",
        f"--account={CPU_ACCOUNT}",
        f"--nodes=1",
        f"--ntasks=1",
        f"--cpus-per-task={thread_count}", # request the desired number of threads
        f"--overcommit",
        f"--mail-type={JOB_MAIL_TYPE}",
        f"--mail-user={JOB_MAIL_USER}",
        # f"--ouptut=some_out_file.txt",
        f"--wrap='scaling_study/slurm_run_serial.sh {SERIAL_TIMING_FILE} {thread_count}'",
      ]
      subprocess.run(command,
      stdout=None,
      check=True,
      stderr=subprocess.STDOUT
      )
    # while jobs still exist
    while subprocess.run(["squeue", "-u", f"{os.getenv('USER')}", "--noheader"], capture_output=True, text=True).stdout.strip():
      print("waiting for jobs to finish...")
      time.sleep(5)
  except KeyboardInterrupt as e:
    subprocess.run(["scancel", "-u", f"{os.getenv('USER')}"])
    raise e
  except subprocess.CalledProcessError as e:
    print(f"\nSERIAL: Command failed for {thread_count} threads.\nError Code: {e.returncode}\noutput:\n{e.stderr}\n")
    subprocess.run(["scancel", "-u", f"{os.getenv('USER')}"])
    raise e
  # get saved timing results
  results = []
  with open(SERIAL_TIMING_FILE, "r", encoding="utf-8") as timing_file:
    csv_reader = csv.DictReader(timing_file)
    for row in csv_reader:
      results.append((row[THREAD_COUNT_HEADER], row[EXECUTION_TIME_HEADER]))
  
  print("Serial done.")
  plot_results(results, label="Serial")

def run_and_plot_omp():
  pass

def run_and_polt_mpi():
  pass

def run_and_plot_cuda():
  pass

def run_and_plot_cuda_mpi():
  print("CUDA with MPI not yet implemented.")


def setup_plot(xlabel, ylabel, log_scale = False):
  plt.xlabel(xlabel)
  plt.ylabel(ylabel)
  if log_scale:
    plt.xscale('log')

def plot_results(results: tuple[int | str, float], label: str):
  plt.plot(results, label=label)

def save_plot(filepath: str = "study_results.png"):
  plt.legend()
  plt.savefig(filepath)

def setup_scratch_data() -> str:
  os.makedirs(SCRATCH_DIR, mode=0o777, exist_ok=True)
  os.makedirs(f"{SCRATCH_DIR}/data/out", mode=0o777, exist_ok=True)
  os.makedirs(f"{SCRATCH_DIR}/timing", mode=0o777, exist_ok=True)
  recursive_chmod(SCRATCH_DIR, 0o777)
  if not os.path.exists(f"{SCRATCH_DIR}/build"):
    shutil.copytree(
      f"build",
      f"{SCRATCH_DIR}/build"
    )
  if not os.path.exists(f"{SCRATCH_DIR}/scaling_study"):
    shutil.copytree(
      f"scaling_study",
      f"{SCRATCH_DIR}/scaling_study"
    )
  shutil.copy(f"data/spotify_short.csv", f"{SCRATCH_DIR}/data/")
  recursive_chmod(SCRATCH_DIR, 0o777)
  pwd = os.getcwd()
  os.chdir(f"{SCRATCH_DIR}")
  return pwd

def main():
  print("Starting scaling study.")
  project_directory = setup_scratch_data()
  setup_plot("Number of Processes / Threads", "Average Execution Time (s)", False)
  run_and_plot_serial()
  run_and_plot_omp()
  run_and_polt_mpi()
  run_and_plot_cuda()
  run_and_plot_cuda_mpi()
  os.chdir(project_directory)
  save_plot()

if __name__ == "__main__":
  main()