#!/bin/bash
#SBATCH --time 0:30:00
#SBATCH --partition=notchpeak-gpu
#SBATCH --account=notchpeak-gpu
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=32
#SBATCH --overcommit
#SBATCH -o scaling_study/logs/slurmjob-%j.out-%N
#SBATCH -e scaling_study/logs/slurmjob-%j.err-%N

module load python/3.10.3
module load gcc/11.2.0
module load cuda/12.5.0
module --latest load openmpi
module --latest load cmake

# file to save timing to is first argument
# thread count to use is second argument
python scaling_study/time_execution.py --target omp --output scaling_study/results/omp_timing.csv