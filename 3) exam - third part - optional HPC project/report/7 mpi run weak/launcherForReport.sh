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
#SBATCH --job-name=WeakScaling_N
module load autoload intelmpi

N_BASE=4000

echo "Inizio test Weak Scaling: Carico costante per core..."

# Array dei processi: 1 e poi da 10 a 100 con step 10
PROCS=(1 $(seq 2 2 24) 25)

for P in "${PROCS[@]}"
do
    # Calcolo N per mantenere il carico costante: N = N_BASE * sqrt(P)
    # Usiamo 'bc' per i calcoli in virgola mobile e arrotondiamo
    N=$(echo "scale=0; sqrt($P) * $N_BASE / 1" | bc -l)

    # Calcolo nodi (P/2 arrotondato per eccesso)
    NODES=$(( (P) ))

    echo "Esecuzione: P=$P, N=$N su $NODES nodi (tpn=1)"

    # Esecuzione
    srun -N $NODES -n $P ./matrix_transformation $N
done

echo "Test completati. Dati salvati"