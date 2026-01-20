#!/bin/bash

#SBATCH --account=tra25_Inginfbo
#SBATCH --partition=g100_usr_prod
#SBATCH --nodes=1 # Richiediamo il massimo numero di nodi previsti
#SBATCH --ntasks-per-node=48 # Massimo numero di task per nodo
#SBATCH -o job.out
#SBATCH -e job.err
#SBATCH --time=00:10:00
#SBATCH --mail-type=ALL
#SBATCH --mail-user=lorenzo@deluca.pro
#SBATCH --job-name=LDLperformanceOptimization
module load autoload intelmpi
N_SIZE=20000

echo "Esecuzione con P=1 (1 Nodo, 1 TPN)"
srun -N 1 -n 1 ./matrix_transformation $N_SIZE

for p in $(seq 4 4 48)
do
    echo "Esecuzione con P=$p (1 Nodo, $p TPN)"
    # srun -N $nodes distribuisce i task sui nodi richiesti
    srun -N 1 -n $p ./matrix_transformation $N_SIZE
done

echo "Test completati. Dati salvati"
