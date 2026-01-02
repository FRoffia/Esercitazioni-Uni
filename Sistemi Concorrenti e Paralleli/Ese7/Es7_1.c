#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>
#include <stdint.h>


int main(int argc, char* argv[]){

    if (argc!=3){   
        printf("sintassi sbagliata -- DIM, N");
        exit(1);
    }

    int DIM, N;
    DIM=atoi(argv[1]); // dimensione matrici quadrate
    N=atoi(argv[2]); // numero threads
    
    uint8_t** A;
    uint8_t** B;
    int** C;
    A = (uint8_t**) malloc(DIM*sizeof(uint8_t*));
    B = (uint8_t**) malloc(DIM*sizeof(uint8_t*));
    C = (int**) malloc(DIM*sizeof(int*));


    for(int i = 0 ; i < DIM; i++){
        A[i] = malloc(DIM*sizeof(uint8_t));
        B[i] = malloc(DIM*sizeof(uint8_t));
        C[i] = malloc(DIM*sizeof(int));

        for(int j = 0; j < DIM; j++){
            A[i][j]= rand()%256;
            B[i][j]= rand()%256;
            C[i][j]= 0;
        }

    }

    double start, end;
    start = omp_get_wtime(); 
	
    int i,j,k,my_rank,tmp;
    # pragma omp parallel  num_threads(N) shared(A,B) private(i,j,k,my_rank,tmp) firstprivate(DIM,N)
    {

        //my_rank=omp_get_thread_num();   
        //printf("thread %d di %d: inizio il calcolo...\n", my_rank, N);

        # pragma omp for
        for(i=0; i<DIM; i++){
            for(j = 0; j < DIM; j++){
                for(k = 0; k < DIM; k++){
                    tmp = (int) A[i][j]*B[j][k];
                    C[i][k] += tmp;
                }
            }
        }
        
        
    }

    end = omp_get_wtime(); 

    /*printf("A:\n");
    for(i = 0; i < DIM; i++){
        for(j = 0; j < DIM; j++){
            printf("%i   ",A[i][j]);
        }
        printf("\n");
    }

    printf("B:\n");
    for(i = 0; i < DIM; i++){
        for(j = 0; j < DIM; j++){
            printf("%i   ",B[i][j]);
        }
        printf("\n");
    }

    printf("C:\n");
    for(i = 0; i < DIM; i++){
        for(j = 0; j < DIM; j++){
            printf("%i   ",C[i][j]);
        }
        printf("\n");
    }*/

    printf("\nTempo di esecuzione: %lf\n",end-start);

}

