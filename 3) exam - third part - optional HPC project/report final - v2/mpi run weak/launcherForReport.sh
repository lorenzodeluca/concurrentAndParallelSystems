#!/bin/bash

#SBATCH --account=tra25_Inginfbo
#SBATCH --partition=g100_usr_prod
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=48
#SBATCH -o job.out
#SBATCH -e job.err
#SBATCH --time=00:10:00
#SBATCH --mail-type=ALL
#SBATCH --mail-user=lorenzo@deluca.pro
#SBATCH --job-name=WeakScaling_N

module load autoload intelmpi

N_BASE=2000   # righe per processo

echo "Inizio test Weak Scaling: righe per processo costanti..."

# Numero di processi
PROCS=(1 $(seq 4 4 48) )

for p in "${PROCS[@]}"
do
    # N cresce linearmente con p
    N=$(awk -v n_base=$N_BASE -v p=$p 'BEGIN { print int(n_base * sqrt(p) + 0.5) }')

    # 1 nodo per tutti i processi, così da togliere la latenza della rete tra i nodi e poter avere un confronto più facile con omp
    NODES=1

    echo "Esecuzione: p=$p, N=$N su $NODES nodi"

    srun -N $NODES -n $p ./matrix_transformation $N
done

echo "Test completati. Dati salvati"
