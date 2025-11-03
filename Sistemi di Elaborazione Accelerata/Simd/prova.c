#include<stdio.h>
#include<immintrin.h>

#define VECTOR_LENGTH 32
#define SSE_DATA_LANE 16
#define DATA_SIZE 1

void print_output(char *A, char *B, int length){
    for(int i = 0; i < VECTOR_LENGTH; i++){
        printf("A[%d]=%d, B[%d]=%d\n",i,A[i],i,B[i]);
    }
}

int main(){
    
    char A[VECTOR_LENGTH] __attribute__((aligned(SSE_DATA_LANE)));
    char B[VECTOR_LENGTH] __attribute__((aligned(SSE_DATA_LANE)));

    __m128i * p_A = (__m128i*) A;
    __m128i * p_B = (__m128i*) B;

    __m128i XMM_SSE_REG;

    for (int i = 0; i<VECTOR_LENGTH; i++){
        A[i]=i;
        B[i]=0;
    }

    printf("\nInput data:\n");
    print_output(A,B,VECTOR_LENGTH);

    for(int i = 0; i < VECTOR_LENGTH*DATA_SIZE/SSE_DATA_LANE; i++){
        XMM_SSE_REG =  _mm_load_si128 (p_A+i);
        _mm_store_si128 (p_B+i,XMM_SSE_REG);
    }

    printf("\nOutput data:\n");

    print_output(A,B,VECTOR_LENGTH);
}

/*
array allocati dinamicamente di dimensione 2^20
misura tempo di copia con rdtsc
array allineato o disallineato:cambia qualcoa
differenze con array di stesse dimensioni di int32
cosa accade se si invalidano tutte le linee di cache
cosa accade se si rendono invalide le linee di cache dopo aver letto un dato?
*/