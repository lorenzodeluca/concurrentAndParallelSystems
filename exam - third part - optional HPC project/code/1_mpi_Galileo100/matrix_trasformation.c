#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stddef.h>
#include <mpi.h>
#include <time.h>

#define DEFAULT_MATRIX_SIZE 2000


int main(int argc, char *argv[])
{   double begin, end, local_elaps,global_elaps;
    int N = DEFAULT_MATRIX_SIZE; // N = matrix rows/cols size

    MPI_Init(&argc, &argv);
     
    int P, rank; // P = number of nodes, rank = node id
    MPI_Comm_size(MPI_COMM_WORLD, &P);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(rank == 0){ // input parameters validation
        //reading command line parameters
        if (argc > 2){
            printf("Usage: %s <matrix size>\n", argv[0]);
            return 1;
        }else if (argc == 2){
            N = atoi(argv[1])
        }
    }

    //sending matrix size to all nodes
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD); // int broadcast from node 0 to all the other nodes

    if(P > N){
        if(rank==0) printf("error: there are more nodes( %d ) than the matrix size %d.\n",P, N);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    //printf("[process %d / %d]\n",rank,size);
    
    // parallel logic starts here, initial sync barrier:
    MPI_Barrier(MPI_COMM_WORLD);
    begin=MPI_Wtime();
        
    // Define this proc(my) vars
    double *MATRIX = NULL;
    int i,j;

     
    if (rank == 0){
        //MATRIX initialization with random values
        MATRIX = malloc(N * N * sizeof(double));
        srand((unsigned)time(NULL));
        for (int i = 0; i < N * N; i++)
            MATRIX[i] = rand() % 100;

        // MATRIX output
        printf("[proc %d] MATRIX:\n", rank);
        for(i=0;i<N;i++){
            for(j=0;j<N;j++)
                printf("%f\t ",MATRIX[i*N + j]);
            printf("\n");
        }
    }

    //scatter of the rows of MATRIX in blocks of N/P rows + 1 overlap row for each block
    // sendcounts[r] - > integer array (of length group size) specifying the number of elements to send to each processor 
    // displs[r] -> where the data for the rank r process starts
    // local_rows -> how many rows the process has
    //
    // only rank 0 calculate the portions... after he sends the data to the nodes
    int *sendcounts = malloc(P * sizeof(int));
    int *displs     = malloc(P * sizeof(int));
    if (rank == 0){
        sendcounts = malloc(P * sizeof(int));
        displs = malloc(P * sizeof(int));

        int rows = N / P;
        int overlap = 1;

        for (int r = 0; r < P; r++){
            int start = r * rows - overlap;
            int end   = (r + 1) * rows + overlap - 1;

            if (start < 0) start = 0; // first node starting row index fix
            if (end >= N) end = N - 1; // last node rows fix 
//TODO: se le righe dell'ultimo processo sono minori di 1, devo anticipare lo start per lui?
//TODO: controllare naming convention vars coerente su tutto il codice e corretta

            int local_rows = end - start + 1;
            sendcounts[r] = local_rows * N;  // number of elements for each node
            displs[r] = start * N;           // where the data for the rank r process starts
        }
    }

    //sending the data to all the nodes
    int my_elems_count;
    MPI_Scatter(sendcounts, 1, MPI_INT, &my_elems_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
    int my_rows = my_elems_count / N;

    double *my_matrix = malloc(my_elems_count * sizeof(double));
    int *my_ris = malloc(my_elems_count * sizeof(int));

    // Scatter of the parts of the matrix calculated before 
    MPI_Scatterv(MATRIX, sendcounts, displs, MPI_DOUBLE,
                 my_matrix, my_elems_count, MPI_DOUBLE,
                 0, MPI_COMM_WORLD);

    //DEBUG: printing sub matrixes
    MPI_Barrier(MPI_COMM_WORLD);
    for (int r = 0; r < P; r++){
        if (rank == r){
            int global_start = displs[rank] / N;
            printf("Rank %d riceve righe globali %d -> %d\n",
                   rank,
                   global_start,
                   global_start + my_rows - 1);

            for (int i = 0; i < my_rows; i++){
                printf("Rank %d riga globale %d: ",
                       rank, global_start + i);
                for (int j = 0; j < N; j++)
                    printf("%3d ", my_matrix[i*N + j]);
                printf("\n");
            }
            printf("\n");
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
    
    // parallel calculus
    for(int r = 0 ; r<my_rows-1; r++){ // TODO: capire come gestire ultima riga assoluta
        for(int c=0;c<N;c++){

            // calculating result for element [r][c]
            int average = 0;
            int elements = 0;
            for(int r2 = r-1 ; r2<=r+1; r2++){
                for(int c2=c-1;c2<=c+1;c2++){
                    if(r2>=0 && r2<my_rows && c2 >= 0 && c2 < N){
                        average += my_matrix[r2*N + c2]
                        elements++;
                    }
                }
            }
            average /= elements;
            if(my_matrix[r*N + c] > average) my_ris[r*N + c] = 1
            else my_ris[r*N + c] = 0
        }
    }

	//results gather into rank 0
    int *ris = NULL;
    if (rank == 0){
        C = malloc(N * N * sizeof(int));
    }
    MPI_Gather(my_ris, my_elems, MPI_INT,
               ris, my_elems, MPI_INT,
               0, MPI_COMM_WORLD);

    if (rank == 0){
        printf("\nRESULT MATRIX:\n");
        for (int i = 0; i < N; i++){
            for (int j = 0; j < N; j++){
                printf("%d ", C[i*N + j]);
            }
                
            printf("\n");
        }
    }    


    MPI_Barrier(MPI_COMM_WORLD);
    end=MPI_Wtime();
    local_elaps= end-begin;
    MPI_Reduce(&local_elaps, &global_elaps,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);
    if (rank == 0){       
        printf("nodes: %d, matrix size: %d, elapsed time: %fs\n",P, N, global_elaps);
    }
    
    
    if (rank == 0){
        printf("\n\n[processo %d] matrice C:\n", rank);
        for(i=0;i<N;i++){
            for(j=0;j<N;j++)
                printf("%d\t ",C[i][j]);
            printf("\n");
        }
    }

    free(my_matrix);
    free(my_ris);
    if (rank == 0) {
        free(MATRIX);
        free(sendcounts);
        free(displs);
        free(C);
    }

    MPI_Finalize();
}
