#!/bin/bash
#SBATCH --time 0:30:00
#SBATCH --partition=notchpeak-gpu
#SBATCH --account=notchpeak-gpu
#SBATCH --nodes=2
#SBATCH --tasks=16
#SBATCH --mail-type=FAIL,BEGIN,END
#SBATCH --mail-user=potatoman.mh@gmail.com
#SBATCH -o slurmjob-%j.out-%N
#SBATCH -e slurmjbo-%j.err-%N

#Set up whatever package we need to run with
module load python/3.10.3
module load gcc/11.2.0
module load cuda/12.5.0
module --latest load openmpi
module --latest load cmake
#set up the scratch directory
export SCRDIR=/scratch/local/$USER/$SLURM_JOBID
mkdir -p $SCRDIR
mkdir -p $SCRDIR/data/out
# mkdir -p data/out
#move input files into scratch directory
cp -r build $SCRDIR/
cp -r scaling_study $SCRDIR/
cp data/spotify_short.csv $SCRDIR/data/spotify_short.csv
cp data/spotify.csv $SCRDIR/data/spotify.csv
cd $SCRDIR
#Run the program with our input
python scaling_study/scaling_study.py
# myprogram < file.input > file.output
#Move files out of working directory and clean up
cp -r data/out $HOME/project/data/out
cd $HOME
rm -rf $SCRDIR
