#!/bin/bash

#SBATCH --account=tra25_Inginfbo
#SBATCH --partition=g100_usr_prod
#SBATCH --nodes=1                    # Sempre 1 nodo per OpenMP
#SBATCH --ntasks=1                   # 1 solo processo (che poi lancia i thread)
#SBATCH --cpus-per-task=48           # Prenotiamo tutti i 48 core del nodo
#SBATCH -o job.out
#SBATCH -e job.err
#SBATCH --time=00:10:00
#SBATCH --job-name=OMP_Strong

N_BASE=2000   # righe per processo

echo "Inizio test weak Scaling OpenMP (N=$N_SIZE)..."

# Iterazione 1: Caso base con 1 thread
echo "Esecuzione con P=1"
./matrix_transformation_omp 1 $N_BASE

# Iterazioni successive: da 4 a 48 a passi di 4
for p in $(seq 4 4 48)
do
    # Calculate N = N_BASE * sqrt(p)
    N=$(awk -v n_base=$N_BASE -v p=$p 'BEGIN { print int(n_base * sqrt(p) + 0.5) }')

    echo "Esecuzione: P=$p, N=$N su 1 nodo (tpn=1)"
    # Lanciamo il programma passando P come primo argomento
    # Usiamo 'export OMP_NUM_THREADS' per sicurezza, anche se il tuo codice usa argv[1]
    ./matrix_transformation_omp $p $N
done

echo "Test completati. Dati salvati"