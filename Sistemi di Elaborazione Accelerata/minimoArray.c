#include<stdio.h>
#include<stdlib.h>
#include<time.h>    
#include<stdint.h>
#include<immintrin.h>
#include<math.h>
#include<x86intrin.h>

#define VECTOR_LENGTH pow(2,16)
#define DATA_SIZE sizeof(char)
#define SSE_DATA_LANE 16

void print_array(char *A){
    for(int i = 0; i < VECTOR_LENGTH; i++){
        printf("%d  ",A[i]);
    }
}



int main(){
    srand(time(NULL));

    signed char* Dati = _mm_malloc(VECTOR_LENGTH*DATA_SIZE,SSE_DATA_LANE);
    for(int i = 0; i < VECTOR_LENGTH; i++){
        Dati[i] = (char) rand() % 255 -127;
    }

    __m128i * p_Dati = (__m128i*) Dati;

    __m128i tempMin;

    __m128i slider,mask,temp1,temp2;

    int min = 100000;


    //=======> Scalare <===================

    uint64_t scalare_start = __rdtsc();

    for(int i = 0; i < VECTOR_LENGTH; i++){
        if(Dati[i] < min){
            min = Dati[i];
        }
    }

    uint64_t scalare_end = __rdtsc();
    printf("Codice Scalare\nMinimo: %i",min);
    printf("\nClock: %lu\n\n", scalare_end-scalare_start);

    //=======> Maschere <===================

    min = 100000;
    uint64_t maschera_start = __rdtsc();

    tempMin =  _mm_load_si128 (p_Dati);

    for(int i = 1; i < VECTOR_LENGTH*DATA_SIZE/SSE_DATA_LANE; i++){
        slider = _mm_load_si128(p_Dati+i);//carichiamo lo slider

        mask = _mm_cmplt_epi8(slider,tempMin);//mask contiene 1 nelle posizioni in cui lo slider è minore del minimo corrente

        temp1 = _mm_and_si128(mask,slider);//valori della nuova sezione che erano minori di quelli precedenti

        temp2 = _mm_andnot_si128(mask,tempMin);//valori della vecchia sezione che erano minori di quelli nuovi

        tempMin = _mm_or_si128(temp1,temp2); //uniamo ottendendo un vettore dei minimi
    }

    for(int i = 0; i < 16; i++){
        signed char c1 = _mm_extract_epi8(tempMin,0);
        if(c1 < min) min = c1;
        tempMin = _mm_srli_si128(tempMin,1);
    }


    uint64_t maschera_end = __rdtsc();
    printf("Maschera\nMinimo: %i",min);
    printf("\nClock: %lu\n\n", maschera_end-maschera_start);

    

    //=======> Blending <===================

    min = 100000;
    uint64_t blending_start = __rdtsc();

    tempMin =  _mm_load_si128 (p_Dati);

    for(int i = 1; i < VECTOR_LENGTH*DATA_SIZE/SSE_DATA_LANE; i++){
        slider = _mm_load_si128(p_Dati+i);//carichiamo lo slider

        mask = _mm_cmplt_epi8(slider,tempMin);//mask contiene 1 nelle posizioni in cui lo slider è minore del minimo corrente

        tempMin = _mm_blendv_epi8 (tempMin,slider,mask); //uniamo ottendendo un vettore dei minimi
    }

    for(int i = 0; i < 16; i++){
        signed char c1 = _mm_extract_epi8(tempMin,0);
        if(c1 < min) min = c1;
        tempMin = _mm_srli_si128(tempMin,1);
    }


    uint64_t blending_end = __rdtsc();
    printf("Maschera\nMinimo: %i",min);
    printf("\nClock: %lu\n\n", blending_end-blending_start);
}

/*

*/