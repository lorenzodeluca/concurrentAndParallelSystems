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

N_SIZE=20000

echo "Inizio test Strong Scaling OpenMP (N=$N_SIZE)..."

# Iterazione 1: Caso base con 1 thread
echo "Esecuzione con P=1"
./matrix_transformation_omp 1 $N_SIZE

# Iterazioni successive: da 4 a 48 a passi di 4
for p in $(seq 4 4 48)
do
    echo "Esecuzione con P=$p"
    # Lanciamo il programma passando P come primo argomento
    # Usiamo 'export OMP_NUM_THREADS' per sicurezza, anche se il tuo codice usa argv[1]
    ./matrix_transformation_omp $p $N_SIZE
done

echo "Test completati. Dati salvati"
