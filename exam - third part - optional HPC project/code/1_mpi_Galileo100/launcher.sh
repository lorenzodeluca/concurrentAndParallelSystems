#!/bin/bash

#SBATCH --account=tra25_Inginfbo
#SBATCH --partition=g100_usr_prod
#SBATCH --nodes=2
#SBATCH --ntasks-per-node=48
#SBATCH -o job.out
#SBATCH -e job.err
#SBATCH --time=00:10:00
#SBATCH --mail-type=ALL
#SBATCH --mail-user=lorenzo@deluca.pro
#SBATCH --job-name=LDL
module load autoload intelmpi
srun ./matrix_trasformation
