#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

#define DEFAULT_MATRIX_SIZE 2000
	
int main(int argc, char* argv[]){   
	double time_paral_begin,time_paral_end,time_seq_begin,time_seq_end;

	time_seq_begin = omp_get_wtime(); 

	//printf("Program started\n");
	fflush(stdout);

	if (argc!=3){   
		printf("Usage: %s <n_proc> <matrix size>\n", argv[0]);
		exit(1);
	}
	
	size_t N; // unsigned 64bit integer
	int P, my_rank;
	P = atoi(argv[1]); //threads number
	N = atoi(argv[2]); //matrix size
	if(P > N)
	{
		printf("error: there are more threads( %d ) than the matrix size %d.\n",P, N);
		exit(1);
	}

	
	
	// inizializzazione vettori:
	
	double *A = NULL;
	int *R = NULL; // A=input/data matrix, R= results matrix. Both shared between threads
	A = malloc(N * N * sizeof(double));
	R = malloc(N * N * sizeof(int));

	srand((unsigned int)time(NULL)); 
	for(size_t r=0;r<N;r++){
		for(size_t c=0;c<N;c++){
			A[r*N+c]=rand()%100;
		}
	}
		
	// debug
	//printf("[Master] A matrix:\n");
	//for(int i=0;i<N;i++){
	//	printf("\t%f\n",A[i]);
	//}
	
	time_seq_end = omp_get_wtime();
    time_paral_begin = omp_get_wtime(); 
	
    # pragma omp parallel  num_threads(P) shared(A,R) private(my_rank) firstprivate(P) 
	{   
		my_rank=omp_get_thread_num();   
        //printf("thread %d / %d started...\n", my_rank, P);
        # pragma omp for
        for(size_t r = 0 ; r<N; r++){
            for(size_t c=0;c<N;c++){

				// calculating result for element [r][c]
				double average = 0;
				size_t elements = 0;
				for(size_t r2 = r-1 ; r2<=r+1; r2++){
					for(size_t c2=c-1;c2<=c+1;c2++){
						if(r2>=0 && r2<N && c2 >= 0 && c2 < N){
							average += A[r2*N + c2];
							elements++;
						}
					}
				}
				average /= elements;
				if(A[r*N + c] > average) R[r*N + c] = 1;
				else R[r*N + c] = 0;
        	}
    	}
	}

    time_paral_end = omp_get_wtime();
    //printf("[Master] result:\n");
	//for (int i = 0; i < N; i++){
	//	for (int j = 0; j < N; j++){
	//		printf("%d ", R[i*N + j]);
	//	}
	//	printf("\n");
	//}

    //csv format output
	//printf("processes;matrix size;seq_elaps;elapsed parallel time\n");       
	double seq_elaps = time_seq_end-time_seq_begin;
	double paral_elaps = time_paral_end-time_paral_begin;
	printf("%d;%d;%f;%f;\n",P, N, seq_elaps,paral_elaps);
	free(A);
	free(R);
    return EXIT_SUCCESS;
}
