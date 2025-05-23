#!/bin/bash
#SBATCH --time 1:00:00
#SBATCH --partition=notchpeak-freecycle
#SBATCH --account=usucs5030
#SBATCH --ntasks-per-node=4
#SBATCH --cpus-per-task=1
#SBATCH --overcommit
#SBATCH -o scaling_study/logs/slurmjob-%j.out-%N
#SBATCH -e scaling_study/logs/slurmjob-%j.err-%N

# If this is not working, change to running on notchpeak-gpu with the option --mem-per-cpu=8000

# number of nodes determined by command line (sbatch --nodes=<number_of_nodes>)

module load python/3.10.3
module load gcc/11.2.0
module load cuda/12.5.0
module --latest load openmpi
module --latest load cmake

# file to save timing to is first argument
# thread count to use is second argument
free -gt
python scaling_study/time_execution.py --target mpi --output "scaling_study/results/mpi_timing_$SLURM_NNODES.csv" --nodes $SLURM_NNODES
