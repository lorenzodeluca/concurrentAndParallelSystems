#!/bin/bash

#SBATCH --account=tra25_Inginfbo
#SBATCH --partition=g100_usr_prod
#SBATCH --nodes=25 # Richiediamo il massimo numero di nodi previsti
#SBATCH --ntasks-per-node=2 # Massimo numero di task per nodo
#SBATCH -o job.out
#SBATCH -e job.err
#SBATCH --time=00:10:00
#SBATCH --mail-type=ALL
#SBATCH --mail-user=lorenzo@deluca.pro
#SBATCH --job-name=LDLperformanceOptimization
module load autoload intelmpi
N_SIZE=20000

echo "Esecuzione con P=50 (25 Nodi, 2 TPN)"
srun -N 25 -n 2 ./matrix_transformation $N_SIZE

echo "Test completati. Dati salvati"