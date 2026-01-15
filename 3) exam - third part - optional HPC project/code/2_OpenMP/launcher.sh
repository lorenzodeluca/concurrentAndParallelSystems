#!/bin/bash

#SBATCH --account=tra25_inginfbo
#SBATCH --partition=g100_usr_prod

#SBATCH -t 00:05:00
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1  # Run a single task per node, more explicit than '-n 1'
#SBATCH -c 24                #  number of CPU cores i.e. OpenMP threads per task
#SBATCH -o OMPjob.out
#SBATCH -e OMPjob.err

for I in 12 24 48; do
  echo "Launching matrix_trasformation_omp  $I"
  srun ./matrix_trasformation_omp $I
done



