#!/bin/bash

#SBATCH --account=tra25_Inginfbo
#SBATCH --partition=g100_usr_prod
#SBATCH --nodes=25
#SBATCH --ntasks-per-node=1
#SBATCH -o job.out
#SBATCH -e job.err
#SBATCH --time=00:10:00
#SBATCH --mail-type=ALL
#SBATCH --mail-user=lorenzo@deluca.pro
#SBATCH --job-name=WeakScaling_N

module load autoload intelmpi

N_BASE=4000   # righe per processo

echo "Inizio test Weak Scaling: righe per processo costanti..."

# Numero di processi
PROCS=(1 $(seq 2 2 24) 25)

for P in "${PROCS[@]}"
do
    # N cresce linearmente con P
    N=$(( N_BASE * P ))

    # 1 processo per nodo
    NODES=$P

    echo "Esecuzione: P=$P, N=$N su $NODES nodi (tpn=1)"

    srun -N $NODES -n $P ./matrix_transformation $N
done

echo "Test completati. Dati salvati"
