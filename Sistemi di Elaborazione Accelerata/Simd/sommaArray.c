#include<stdio.h>
#include<stdlib.h>
#include<time.h>    
#include<stdint.h>
#include<immintrin.h>
#include<math.h>
#include<x86intrin.h>

#define VECTOR_LENGTH 4128
#define DATA_SIZE sizeof(char)
#define SSE_DATA_LANE 16

void print_array(char *A){
    for(int i = 0; i < VECTOR_LENGTH; i++){
        printf("%d  ",A[i]);
    }
}

int main(){
    srand(time(NULL));

    unsigned char* data = (char*) _mm_malloc(VECTOR_LENGTH*DATA_SIZE,SSE_DATA_LANE);

    for(int i = 0; i < VECTOR_LENGTH; i++){
        //data[i] = (char) rand() % 256;
        //data[i] = 1;
        data[i] = 255;
    }

    __m128i * p_data = (__m128i*) data;

    __m128i slider;

    __m128i _16_bit_1, _16_bit_2 , _16_bit_intermediate, _16_bit_buffer = _mm_setzero_si128();
    __m128i _32_bit_1, _32_bit_2, _32_bit_intermediate, _32_bit_buffer = _mm_setzero_si128();

    __m128i zero = _mm_setzero_si128();

    int somma = 0;

    //==============<< Somma con add >>==================================
    printf("Utilizzando add:\n");
    
    uint64_t start = __rdtsc();

    for(int i = 0; i < VECTOR_LENGTH*DATA_SIZE/SSE_DATA_LANE; i++){
        slider =  _mm_load_si128 (p_data+i);
        
        _16_bit_1 = _mm_unpacklo_epi8(slider,zero);
        _16_bit_2 = _mm_unpackhi_epi8(slider,zero);

        /*for(int j = 0; j < 16; j++){
            unsigned char c1 = _mm_extract_epi8(slider,0);
            printf("Slider i:%i,  j:%i|  c:%i\n",i,j,c1);
            slider = _mm_srli_si128(slider,1);
        }*/

        if(i%128 < 127){
            _16_bit_buffer = _mm_add_epi16(_16_bit_buffer,_16_bit_1);
            _16_bit_buffer = _mm_add_epi16(_16_bit_buffer,_16_bit_2);
            
        } else{
            _16_bit_buffer = _mm_add_epi16(_16_bit_buffer,_16_bit_1);
            _16_bit_buffer = _mm_add_epi16(_16_bit_buffer,_16_bit_2);

            _32_bit_1 = _mm_unpacklo_epi16(_16_bit_buffer,zero);
            _32_bit_2 = _mm_unpackhi_epi16(_16_bit_buffer,zero);

            _32_bit_buffer = _mm_add_epi32(_32_bit_buffer,_32_bit_1);
            _32_bit_buffer = _mm_add_epi32(_32_bit_buffer,_32_bit_2);

            _16_bit_buffer = _mm_setzero_si128();
        }

        /*for(int j = 0; j < 8; j++){
            int c1 = _mm_extract_epi16(_16_bit_buffer,0);
            printf("Buffer16 i:%i,  j:%i|  c:%i\n",i,j,c1);
            slider = _mm_srli_si128(slider,2);
        }*/

    }

    //per aggiustare per numeri non multipli di 128
    _32_bit_1 = _mm_unpacklo_epi16(_16_bit_buffer,zero);
    _32_bit_2 = _mm_unpackhi_epi16(_16_bit_buffer,zero);

    _32_bit_buffer = _mm_add_epi32(_32_bit_buffer,_32_bit_1);
    _32_bit_buffer = _mm_add_epi32(_32_bit_buffer,_32_bit_2);

    if(VECTOR_LENGTH > 1024){
        for(int i = 0; i < 4; i++){
            int tmp = _mm_extract_epi32(_32_bit_buffer,0); 
            somma += tmp;
            //printf("Somma32[%i]: %i\n",i,tmp);
            _32_bit_buffer = _mm_srli_si128(_32_bit_buffer,4);
        }
    } else {
        for(int i = 0; i < 8; i++){
            int tmp = _mm_extract_epi16(_16_bit_buffer,0); 
            somma += tmp;
            //printf("Somma16[%i]: %i\n",i,tmp);
            _16_bit_buffer = _mm_srli_si128(_16_bit_buffer,2);

        }
    }


    uint64_t end = __rdtsc();
    printf("Somma add: %i",somma);
    printf("\nClock: %lu\n\n\n\n", end-start);


    

    //==============<< Somma con hadd >>==================================
    printf("Utilizzando hadd:\n");
    
    somma = 0;
    start = __rdtsc();
    _16_bit_buffer = _mm_setzero_si128();
    _32_bit_buffer = _mm_setzero_si128();


    for(int i = 0; i < VECTOR_LENGTH*DATA_SIZE/SSE_DATA_LANE; i++){
        slider =  _mm_load_si128 (p_data+i);
        
        _16_bit_1 = _mm_unpacklo_epi8(slider,zero);
        _16_bit_2 = _mm_unpackhi_epi8(slider,zero);

        /*for(int j = 0; j < 16; j++){
            unsigned char c1 = _mm_extract_epi8(slider,0);
            printf("Slider i:%i,  j:%i|  c:%i\n",i,j,c1);
            slider = _mm_srli_si128(slider,1);
        }*/

        if(i%128 < 127){
            _16_bit_intermediate = _mm_hadd_epi16(_16_bit_1,_16_bit_2);
            _16_bit_buffer = _mm_add_epi16(_16_bit_buffer,_16_bit_intermediate);
            
        } else{
            _16_bit_intermediate = _mm_hadd_epi16(_16_bit_1,_16_bit_2);
            _16_bit_buffer = _mm_add_epi16(_16_bit_buffer,_16_bit_intermediate);

            _32_bit_1 = _mm_unpacklo_epi16(_16_bit_buffer,zero);
            _32_bit_2 = _mm_unpackhi_epi16(_16_bit_buffer,zero);

            _32_bit_intermediate = _mm_hadd_epi32(_32_bit_1,_32_bit_2);
            _32_bit_buffer = _mm_add_epi32(_32_bit_buffer,_32_bit_intermediate);

            _16_bit_buffer = _mm_setzero_si128();
        }

        /*for(int j = 0; j < 8; j++){
            int c1 = _mm_extract_epi16(_16_bit_buffer,0);
            printf("Buffer16 i:%i,  j:%i|  c:%i\n",i,j,c1);
            slider = _mm_srli_si128(slider,2);
        }*/

    }

    //per aggiustare per numeri non multipli di 128
    _32_bit_1 = _mm_unpacklo_epi16(_16_bit_buffer,zero);
    _32_bit_2 = _mm_unpackhi_epi16(_16_bit_buffer,zero);

    _32_bit_intermediate = _mm_hadd_epi32(_32_bit_1,_32_bit_2);
    _32_bit_buffer = _mm_add_epi32(_32_bit_buffer,_32_bit_intermediate);

    if(VECTOR_LENGTH > 1024){
        for(int i = 0; i < 4; i++){
            int tmp = _mm_extract_epi32(_32_bit_buffer,0); 
            somma += tmp;
            //printf("Somma32[%i]: %i\n",i,tmp);
            _32_bit_buffer = _mm_srli_si128(_32_bit_buffer,4);
        }
    } else {
        for(int i = 0; i < 8; i++){
            int tmp = _mm_extract_epi16(_16_bit_buffer,0); 
            somma += tmp;
            //printf("Somma16[%i]: %i\n",i,tmp);
            _16_bit_buffer = _mm_srli_si128(_16_bit_buffer,2);

        }
    }


    end = __rdtsc();
    printf("Somma hadd: %i",somma);
    printf("\nClock: %lu\n\n\n\n", end-start);

}