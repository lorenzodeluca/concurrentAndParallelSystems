#!/bin/bash

#SBATCH --account=tra25_Inginfbo
#SBATCH --partition=g100_usr_prod
#SBATCH --nodes=10 # Richiediamo il massimo numero di nodi previsti
#SBATCH --ntasks-per-node=48 # Massimo numero di task per nodo
#SBATCH -o job.out
#SBATCH -e job.err
#SBATCH --time=00:10:00
#SBATCH --mail-type=ALL
#SBATCH --mail-user=lorenzo@deluca.pro
#SBATCH --job-name=LDLperformanceOptimization
module load autoload intelmpi
N_SIZE=8000

# ---------------------------------------------------------
# FASE 1: 1 Nodo, Task per nodo (TPN) crescenti
# ---------------------------------------------------------
echo "Inizio Fase 1: Scalabilità intra-nodo..."
for tpn in 1 2 4 8 12 24 48
do
    echo "Esecuzione con P=$tpn (1 Nodo, $tpn TPN)"
    # srun -N 1 forza l'uso di un solo nodo
    srun -N 1 -n $tpn ./matrix_trasformation $N_SIZE
done

# ---------------------------------------------------------
# FASE 2: Nodi crescenti (2 -> 10), TPN fisso a 48
# ---------------------------------------------------------
echo "Inizio Fase 2: Scalabilità inter-nodo..."
for nodes in {2..10}
do
    total_tasks=$((nodes * 48))
    echo "Esecuzione con P=$total_tasks ($nodes Nodi, 48 TPN)"
    # srun -N $nodes distribuisce i task sui nodi richiesti
    srun -N $nodes -n $total_tasks ./matrix_trasformation $N_SIZE
done

echo "Test completati. Dati salvati"
