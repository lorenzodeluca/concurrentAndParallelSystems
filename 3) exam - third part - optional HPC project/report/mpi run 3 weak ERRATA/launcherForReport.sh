#!/bin/bash

#SBATCH --account=tra25_Inginfbo
#SBATCH --partition=g100_usr_prod
#SBATCH --nodes=2 # Richiediamo il massimo numero di nodi previsti
#SBATCH --ntasks-per-node=20 # Massimo numero di task per nodo
#SBATCH -o job.out
#SBATCH -e job.err
#SBATCH --time=00:10:00
#SBATCH --mail-type=ALL
#SBATCH --mail-user=lorenzo@deluca.pro
#SBATCH --job-name=WeakScaling_N
module load autoload intelmpi

P_TOT=40

# ---------------------------------------------------------
# LOOP: N aumenta da 2000 a 42000 con step di 4000
# ---------------------------------------------------------
echo "Inizio test di scalabilità con P=$P_TOT..."

for n in $(seq 2000 4000 42000)
do
    echo "Esecuzione con N=$n (P=$P_TOT)"
    srun -N 2 -n $P_TOT ./matrix_trasformation $n
done

echo "Test completati. Dati salvati"
