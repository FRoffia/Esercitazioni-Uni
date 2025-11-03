#include<stdio.h>
#include<stdint.h>
#include<immintrin.h>
#include<math.h>
#include<x86intrin.h>

#define VECTOR_LENGTH pow(2,20)
#define DATA_SIZE sizeof(char)
#define SSE_DATA_LANE 16
#define DATA_SIZE 1

void print_output(char *A, char *B, int length){
    for(int i = 0; i < VECTOR_LENGTH; i++){
        printf("A[%d]=%d, B[%d]=%d\n",i,A[i],i,B[i]);
    }
}

int main(){
    
    char* A = _mm_malloc(VECTOR_LENGTH*DATA_SIZE,SSE_DATA_LANE);
    char* B = _mm_malloc(VECTOR_LENGTH*DATA_SIZE,SSE_DATA_LANE);

    __m128i * p_A = (__m128i*) A;
    __m128i * p_B = (__m128i*) B;

    __m128i XMM_SSE_REG;

    for (int i = 0; i<VECTOR_LENGTH; i++){
        A[i]=i;
        B[i]=0;
    }

    uint64_t clock_counter_start = __rdtsc();
    
    for(int i = 0; i < VECTOR_LENGTH*DATA_SIZE/SSE_DATA_LANE; i++){
        XMM_SSE_REG =  _mm_load_si128 (p_A+i);
        _mm_store_si128 (p_B+i,XMM_SSE_REG);
    }
    
    uint64_t clock_counter_end = __rdtsc();
    printf("\nElapsed clocks: %lu\n", clock_counter_end-clock_counter_start);

    //print_output(A,B,VECTOR_LENGTH);
}

/*
array allocati dinamicamente di dimensione 2^20
misura tempo di copia con rdtsc
array allineato o disallineato:cambia qualcoa
differenze con array di stesse dimensioni di int32
cosa accade se si invalidano tutte le linee di cache
cosa accade se si rendono invalide le linee di cache dopo aver letto un dato?
*/