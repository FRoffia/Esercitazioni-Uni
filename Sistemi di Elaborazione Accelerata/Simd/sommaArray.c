#include<stdio.h>
#include<stdlib.h>
#include<time.h>    
#include<stdint.h>
#include<immintrin.h>
#include<math.h>
#include<x86intrin.h>

#define VECTOR_LENGTH 16348
#define DATA_SIZE sizeof(char)
#define SSE_DATA_LANE 128/(DATA_SIZE*8)

void print_array(char *A){
    for(int i = 0; i < VECTOR_LENGTH; i++){
        printf("%d  ",A[i]);
    }
}



int main(){
    srand(time(NULL));

    unsigned char* data = (char*) _mm_malloc(VECTOR_LENGTH*DATA_SIZE,SSE_DATA_LANE);

    for(int i = 0; i < VECTOR_LENGTH; i++){
        data[i] = (char) rand() % 256;
    }

    __m128i * p_data = (__m128i*) data;

    __m128i temp0,slider,mask,temp1,temp2;
    
    uint64_t start = __rdtsc();

    for(int i = 0; i < VECTOR_LENGTH*DATA_SIZE/SSE_DATA_LANE; i++){
        temp0 =  _mm_load_si128 (p_data);
        
        
    }

    char * Maschera = (char*) &mask;
    
    for(int i = 0; i < VECTOR_LENGTH; i++){
        if(Maschera[i] < min){
            min = Maschera[i];
        }
    }

    uint64_t end = __rdtsc();
    printf("Somma: %i",min);
    printf("\nClock: %lu\n\n\n\n", end-start);

    //print_output(A,B,VECTOR_LENGTH);
}

/*

*/