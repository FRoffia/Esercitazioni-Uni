#include<stdio.h>
#include<stdlib.h>
#include<time.h>    
#include<stdint.h>
#include<immintrin.h>
#include<math.h>
#include<x86intrin.h>

#define VECTOR_LENGTH 64
#define DATA_SIZE sizeof(char)
#define SSE_DATA_LANE 16

void print_array(char *A){
    for(int i = 0; i < VECTOR_LENGTH; i++){
        printf("%d  ",A[i]);
    }
}



int main(){
    srand(time(NULL));

    signed char* Maschere = _mm_malloc(VECTOR_LENGTH*DATA_SIZE,SSE_DATA_LANE);

    for(int i = 0; i < VECTOR_LENGTH; i++){
        Maschere[i] = (char) rand() % 255 -127;
    }

    printf("Codice scalare:\n");
    for(int i = 0; i < VECTOR_LENGTH; i++){
        printf("%d  ",Maschere[i]);
    }
    printf("\n");

    int min = 100000;

    uint64_t scalare_start = __rdtsc();

    

    for(int i = 0; i < VECTOR_LENGTH; i++){
        if(Maschere[i] < min){
            min = Maschere[i];
        }
    }

    uint64_t scalare_end = __rdtsc();
    printf("Minimo: %i",min);
    printf("\nClock: %lu\n\n\n\n", scalare_end-scalare_start);

    __m128i * p_Maschere = (__m128i*) Maschere;

    __m128i tempMin;

    tempMin =  _mm_load_si128 (p_Maschere);

    __m128i slider,mask,temp1,temp2;

    min = 100000;

    printf("Maschera:\n");
    print_array(Maschere);
    printf("\n");

    
    uint64_t maschera_start = __rdtsc();

    for(int i = 1; i < VECTOR_LENGTH*DATA_SIZE/SSE_DATA_LANE; i++){
        slider = _mm_load_si128(p_Maschere+i);
        mask = _mm_cmplt_epi8(slider,tempMin);
        temp1 = _mm_and_si128(mask,slider);//valori della nuova sezione che erano minori di quelli precedenti
        temp2 = _mm_andnot_si128(mask,temp1);//valori della vecchia sezione che erano minori di quelli nuovi
        tempMin = _mm_or_si128(temp1,temp2); //uniamo ottendendo un vettore dei minimi
    }

    char * Maschera = (char*) &mask;
    
    for(int i = 0; i < VECTOR_LENGTH; i++){
        if(Maschera[i] < min){
            min = Maschera[i];
        }
    }

    uint64_t maschera_end = __rdtsc();
    printf("Minimo: %i",min);
    printf("\nClock: %lu\n\n\n\n", maschera_end-maschera_start);

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