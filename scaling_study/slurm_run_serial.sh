#!/bin/bash
#SBATCH -o serial_-%j.out-%N
#SBATCH -e serial_-%j.err-%N

module load python/3.10.3
module load gcc/11.2.0
module load cuda/12.5.0
module --latest load openmpi
module --latest load cmake

# file to save timing to is first argument
# thread count to use is second argument

python time_execution.py --target serial --timing-output-file $1 --thread-count $2