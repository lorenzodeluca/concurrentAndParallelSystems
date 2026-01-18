#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stddef.h>
#include <mpi.h>
#include <time.h>

#define DEFAULT_MATRIX_SIZE 2000
#define ENABLE_DEBUG_OUTPUTS 0

int main(int argc, char *argv[]){   
    double time_paral_begin,time_paral_end,time_seq_begin,time_seq_end,local_elaps,seq_elaps,global_paral_elaps;
    int N = DEFAULT_MATRIX_SIZE; // N = matrix rows/cols size

    MPI_Init(&argc, &argv);
     
    int P, rank; // P = number of processes, rank = process id
    MPI_Comm_size(MPI_COMM_WORLD, &P);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //matrix init
    double *A = NULL; //data matrix

    // scatter/gather vars
    int *sendcounts = NULL; //integer array, number of values to send to each processor(including additional rows("overlapping rows") needed for calculations)
    int *displs = NULL; //integer array, Entry i specifies the displacement relative to sendbuf from which to take the outgoing data to process i
    int *valid_rows = NULL; //number of rows that this process needs to calculate(without overlapping rows)
    int *start_row_by_process = NULL; //starting row in the original matrix
    int *recvcounts = NULL; //for the gatherv, without additional overlapping rows
    int *recvdispls = NULL; //for the gatherv, without additional overlapping rows

    if(rank == 0){ // input parameters validation
        time_seq_begin = MPI_Wtime();

        //reading command line parameters
        if (argc > 2){
            printf("Usage: %s <matrix size>\n", argv[0]);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }else if (argc == 2){
            N = atoi(argv[1]);
        }

        //A initialization with random values
        A = malloc(N * N * sizeof(double));
        srand((unsigned)time(NULL));
        for (int i = 0; i < N * N; i++)
            A[i] = rand() % 100;

        // A output
        if(ENABLE_DEBUG_OUTPUTS){
            printf("[proc %d] T:\n", rank);
            for(int i=0;i<N;i++){
                for(int j=0;j<N;j++)
                    printf("%f\t ",A[i*N + j]);
                printf("\n");
            }
        }

        ///////// --- SCATTER/GATHER preparation---

        //scatter of the rows of A in blocks of N/P rows + 1 overlap row for each block
        // sendcounts[r] - > integer array (of length group size) specifying the number of elements to send to each processor 
        // displs[r] -> where the data for the rank r process starts
        // local_rows -> how many rows the process has
        //
        // only rank 0 calculate the portions... after he sends the data to the processes
        //calculating which rows each process needs
        sendcounts = malloc(P * sizeof(int));
        displs = malloc(P * sizeof(int));
        valid_rows = malloc(P * sizeof(int));
        start_row_by_process = malloc(P * sizeof(int));
        recvcounts   = malloc(P * sizeof(int));//containing the number of elements that are received from each process  
        recvdispls   = malloc(P * sizeof(int));//displacement relative to recvbuf at which to place the incoming data from process r 

        int rows_per_node = N / P; // standard number of rows for each process
        int remaining_rows  = N % P; // needed if there is an uneven number of rows for each process
        int recv_disp = 0;

        int current = 0;
        for (int r = 0; r < P; r++) {
            int rows = rows_per_node; 
            if(r<remaining_rows){// the first processes get the uneven rows
                rows+=1;
            }


            int start = current;
            int end   = current+rows-1;

            int send_start = start - 1;  //adding one overlap rows at the beginning of the rows for this 
            if(start == 0){ //first process
                send_start = start;
            }

            int send_end   = end + 1;
            if(end == N - 1){//last process
                send_end = end;
            }

            valid_rows[r]=rows; //number of rows that this process must output(without overlap rows needed for doing the calculations)
            start_row_by_process[r]=start;//first row that this process must edit

            //scatter vars
            sendcounts[r]=(send_end-send_start+1)*N;//number of rows * columns for each row
            displs[r]=send_start * N; //starting position for the first element for the r process 

            //gather vars
            recvcounts[r]=rows * N;
            recvdispls[r]=recv_disp;
            recv_disp+=recvcounts[r];

            current+=rows;
        }
        time_seq_end = MPI_Wtime();
    }

    // parallel logic starts here, initial sync barrier:
    MPI_Barrier(MPI_COMM_WORLD);
    time_paral_begin=MPI_Wtime();

    //sending matrix size to all processes
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD); // int broadcast from process 0 to all the other processes

    if(P > N){
        if(rank==0) printf("error: there are more processes( %d ) than the matrix size %d.\n",P, N);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    //printf("[process %d / %d]\n",rank,size);

    
    ///////// --- SCATTER---

    // from sendcounts(array from rank 0) to my_elems_count
    int my_elems_count;
    MPI_Scatter(sendcounts, 1, MPI_INT, &my_elems_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
    int my_rows = my_elems_count / N;

    int my_start_row;
    int my_valid_rows;
    MPI_Scatter(start_row_by_process, 1, MPI_INT,
                &my_start_row, 1, MPI_INT,
                0, MPI_COMM_WORLD);
    MPI_Scatter(valid_rows, 1, MPI_INT,
                &my_valid_rows, 1, MPI_INT,
                0, MPI_COMM_WORLD);

    // from A(from rank 0) to my_matrix
    double *my_matrix = malloc(my_elems_count * sizeof(double)); //includes overlaps
    MPI_Scatterv(A, sendcounts, displs, MPI_DOUBLE,
                 my_matrix, my_elems_count, MPI_DOUBLE,
                 0, MPI_COMM_WORLD);

    //DEBUG: printing sub matrixes
    if(ENABLE_DEBUG_OUTPUTS){
        MPI_Barrier(MPI_COMM_WORLD);
        for (int r = 0; r < P; r++){
            if (rank == r){
                int global_start = my_start_row;
                printf("Rank %d rows %d -> %d\n",
                    rank,
                    global_start,
                    global_start + my_rows - 1);

                for (int i = 0; i < my_rows; i++){
                    printf("Rank %d row %d: ",
                        rank, global_start + i);
                    for (int j = 0; j < N; j++)
                        printf("%6.2f ", my_matrix[i*N + j]);
                    printf("\n");
                }
                printf("\n");
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }
    }
    
    ///////// --- parallel calculations: each node calculate its part of the result matrix---
    int *my_ris = malloc(my_valid_rows*N*sizeof(int));
    int r_without_overlap, offset=0;//to remove the my_matrix beginning overlap if present
    if(my_start_row!=0){ //if my_matrix contains a overlap row at the beginning i ignore it for calculations 
        offset=1;
    }

    for(int r = 0 ; r<my_valid_rows; r++){
         r_without_overlap = r+offset;
        for(int c=0;c<N;c++){
            // calculating result for element [r][c]
            double average = 0;
            int elements = 0;
            for(int r2 = r_without_overlap-1 ; r2<=r_without_overlap+1; r2++){
                for(int c2=c-1;c2<=c+1;c2++){
                    if(r2>=0 && r2<my_rows && c2 >= 0 && c2 < N){
                        average += my_matrix[r2*N + c2];
                        elements++;
                    }
                }
            }
            average /= elements;
            if(my_matrix[r_without_overlap*N + c] > average) my_ris[r*N + c] = 1;
            else my_ris[r*N + c] = 0;
        }
    }

    ///////// --- GATHER---

	//results gather into rank 0
    int *T = NULL; //results matrix
    if (rank == 0){
        T = malloc(N * N * sizeof(int));
    }

    //MPI_Gatherv: doesnt contain overlap rows
    MPI_Gatherv(my_ris, my_valid_rows * N, MPI_INT,
                T, recvcounts, recvdispls, MPI_INT,
                0, MPI_COMM_WORLD);

    if (rank == 0){
        if(ENABLE_DEBUG_OUTPUTS){
            printf("\nRESULT MATRIX T:\n");
            for (int i = 0; i < N; i++){
                for (int j = 0; j < N; j++){
                    printf("%d ", T[i*N + j]);
                }
                    
                printf("\n");
            }
        }
    }    


    MPI_Barrier(MPI_COMM_WORLD);
    time_paral_end=MPI_Wtime();
    local_elaps= time_paral_end-time_paral_begin;
    MPI_Reduce(&local_elaps, &global_paral_elaps,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);
    if (rank == 0){
        seq_elaps = time_seq_end-time_seq_begin;
        //csv format output
        //printf("processes;matrix size;seq_elaps;elapsed parallel time\n");       
        printf("%d;%d;%f;%f;\n",P, N, seq_elaps,global_paral_elaps);
    }

    free(my_matrix);
    free(my_ris);
    if (rank == 0) {
        free(A);
        free(sendcounts);
        free(displs);
        free(T);
        free(valid_rows);
        free(start_row_by_process);
        free(recvcounts);
        free(recvdispls);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
