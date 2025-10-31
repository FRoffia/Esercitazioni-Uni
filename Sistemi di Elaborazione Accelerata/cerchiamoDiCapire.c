#include<stdio.h>
#include<immintrin.h>

#define VECTOR_LENGTH 200
#define SSE_DATA_LANE 16
#define DATA_SIZE 1

void print_output(char *A, char *B, int length){
    for(int i = 0; i < VECTOR_LENGTH; i++){
        printf("A[%d]=%d, B[%d]=%d\n",i,A[i],i,B[i]);
    }
}

int main(){
    
    char A[VECTOR_LENGTH] __attribute__((aligned(SSE_DATA_LANE)));
    //char B[VECTOR_LENGTH] __attribute__((aligned(SSE_DATA_LANE)));

    __m128i * p_A = (__m128i*) A;
    //__m128i * p_B = (__m128i*) B;

    __m128i XMM_SSE_REG;

    for (int i = 0; i<VECTOR_LENGTH; i++){
        A[i]=i;
        //B[i]=0;
    }

    //printf("\nInput data:\n");
    //print_output(A,B,VECTOR_LENGTH);

    for(int i = 0; i < VECTOR_LENGTH*DATA_SIZE/16; i++){//qui il calcolo serve perchè io prendo 16 byte alla volta, e devo capire quanti dati ho in byte.
        XMM_SSE_REG =  _mm_load_si128 (p_A+i);//mi sposto sul vettorone di 128 bit alla volta.
        unsigned char c1;
        for(int j = 0; j < 16/DATA_SIZE; j++){
            c1 = (unsigned char) _mm_extract_epi8(XMM_SSE_REG,0);
            printf("i:%i, j:%i, c1:%i\n",i,j,c1);
            XMM_SSE_REG = _mm_srli_si128(XMM_SSE_REG,1);
        }
        //_mm_store_si128 (p_B+i,XMM_SSE_REG);
    }

    //printf("\nOutput data:\n");

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