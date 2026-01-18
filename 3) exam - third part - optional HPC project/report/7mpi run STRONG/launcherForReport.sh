#!/bin/bash

#SBATCH --account=tra25_Inginfbo
#SBATCH --partition=g100_usr_prod
#SBATCH --nodes=25 # Richiediamo il massimo numero di nodi previsti
#SBATCH --ntasks-per-node=1 # Massimo numero di task per nodo
#SBATCH -o job.out
#SBATCH -e job.err
#SBATCH --time=00:10:00
#SBATCH --mail-type=ALL
#SBATCH --mail-user=lorenzo@deluca.pro
#SBATCH --job-name=LDLperformanceOptimization
module load autoload intelmpi
N_SIZE=20000


for nodes in {1..25}
do
    total_tasks=$((nodes))
    echo "Esecuzione con P=$total_tasks ($nodes Nodi, 1 TPN)"
    # srun -N $nodes distribuisce i task sui nodi richiesti
    srun -N $nodes -n $total_tasks ./matrix_transformation $N_SIZE
done

echo "Test completati. Dati salvati"
