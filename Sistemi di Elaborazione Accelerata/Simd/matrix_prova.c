#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include<immintrin.h>
#include <time.h> //conteggio tempo



int main() {
    int width = 3840;
    int height = 5120;
    
    int16_t* matrix = (int16_t*)_mm_malloc(width * height * sizeof(int16_t), 64);
    int16_t* out = (int16_t*)_mm_malloc(width * height * sizeof(int16_t), 64);
    
    // Inizializza con id sequenziale
    for (int i = 0; i < width * height; i++) {
        matrix[i] = i;
    }
    
    // Stampa tutta la matrice
    /*for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            printf("%6d ", matrix[i * width + j]);
        }
        printf("\n");
    }*/

    clock_t tic = clock(); //inizio nostro codice

    transpose_matrix(matrix,out,width,height);

    clock_t toc = clock(); //inizio nostro codice

    printf("Tempo(%ix%i):    %.3f seconds\n",width,height, (double)(toc - tic) / CLOCKS_PER_SEC);

    printf("\n\n\n\n");

    /*
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            printf("%6d ", out[i * width + j]);
        }
        printf("\n");
    }*/
    
    free(matrix);
    return 0;
}

void transpose_matrix(int16_t* matrix, int16_t* out, int width, int height){
    __m512i block_in[32];
    __m512i block_out[32];

    for (int i = 0; i < (width*height)/32; i += 32) {
        int x_0 = i%width;
        int y_0 = (i/width)*32;
        
        //carico le 32 righe (32 x 32)
        for(int k = 0; k < 32; k++){
            block_in[k] = _mm512_load_si512((const __m512i*)(matrix+x_0+(y_0+k)*width));
        }

        transpose_block(block_in,block_out);

        //salvo le 32 righe trasposte (32 x 32)
        for(int k = 0; k < 32; k++){
            _mm512_store_si512(out+(y_0+(x_0+k)*width),block_out[k]);
        }
    }
}

void transpose_block(__m512i block_in[32],__m512i block_out[32]){
    __m512i mask_0h = _mm512_set_epi16(63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i mask_0l = _mm512_set_epi16(47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);

    __m512i mask_1h = _mm512_set_epi16(63,62,61,60,59,58,57,56,47,46,45,44,43,42,41,40,31,30,29,28,27,26,25,24,15,14,13,12,11,10,9,8);
    __m512i mask_1l = _mm512_set_epi16(55,54,53,52,51,50,49,48,39,38,37,36,35,34,33,32,23,22,21,20,19,18,17,16,7,6,5,4,3,2,1,0);

    __m512i mask_2h = _mm512_set_epi16(63,62,61,60,55,54,53,52,47,46,45,44,39,38,37,36,31,30,29,28,23,22,21,20,15,14,13,12,7,6,5,4);
    __m512i mask_2l = _mm512_set_epi16(59,58,57,56,51,50,49,48,43,42,41,40,35,34,33,32,27,26,25,24,19,18,17,16,11,10,9,8,3,2,1,0);

    __m512i mask_3h = _mm512_set_epi16(63,62,59,58,55,54,51,50,47,46,43,42,39,38,35,34,31,30,27,26,23,22,19,18,15,14,11,10,7,6,3,2);
    __m512i mask_3l = _mm512_set_epi16(61,60,57,56,53,52,49,48,45,44,41,40,37,36,33,32,29,28,25,24,21,20,17,16,13,12,9,8,5,4,1,0);

    __m512i mask_4h = _mm512_set_epi16(63,61,59,57,55,53,51,49,47,45,43,41,39,37,35,33,31,29,27,25,23,21,19,17,15,13,11,9,7,5,3,1);
    __m512i mask_4l = _mm512_set_epi16(62,60,58,56,54,52,50,48,46,44,42,40,38,36,34,32,30,28,26,24,22,20,18,16,14,12,10,8,6,4,2,0);

    __m512i t_0h[16];
    __m512i t_0l[16];

    __m512i t_1hh[8];
    __m512i t_1hl[8];
    __m512i t_1lh[8];
    __m512i t_1ll[8];

    __m512i t_2hhh[4];
    __m512i t_2hhl[4];
    __m512i t_2hlh[4];
    __m512i t_2hll[4];
    __m512i t_2lhh[4];
    __m512i t_2lhl[4];
    __m512i t_2llh[4];
    __m512i t_2lll[4];

    __m512i t_3hhhh[2];
    __m512i t_3hhhl[2];
    __m512i t_3hhlh[2];
    __m512i t_3hhll[2];
    __m512i t_3hlhh[2];
    __m512i t_3hlhl[2];
    __m512i t_3hllh[2];
    __m512i t_3hlll[2];
    __m512i t_3lhhh[2];
    __m512i t_3lhhl[2];
    __m512i t_3lhlh[2];
    __m512i t_3lhll[2];
    __m512i t_3llhh[2];
    __m512i t_3llhl[2];
    __m512i t_3lllh[2];
    __m512i t_3llll[2];

    //giro con maschere 0
    for(int i = 0; i < 16; i++){
        t_0h[i] = _mm512_permutex2var_epi16(block_in[2*i],mask_0h,block_in[2*i+1]);
        t_0l[i] = _mm512_permutex2var_epi16(block_in[2*i],mask_0l,block_in[2*i+1]);
    }

    //giro con maschere 1
    for(int i = 0; i < 8; i++){
        t_1hh[i] = _mm512_permutex2var_epi16(t_0h[2*i],mask_1h,t_0h[2*i+1]);
        t_1hl[i] = _mm512_permutex2var_epi16(t_0h[2*i],mask_1l,t_0h[2*i+1]);

        t_1lh[i] = _mm512_permutex2var_epi16(t_0l[2*i],mask_1h,t_0l[2*i+1]);
        t_1ll[i] = _mm512_permutex2var_epi16(t_0l[2*i],mask_1l,t_0l[2*i+1]);
    }

    //giro con maschere 2
    for(int i = 0; i < 4; i++){
        t_2hhh[i] = _mm512_permutex2var_epi16(t_1hh[2*i],mask_2h,t_1hh[2*i+1]);
        t_2hhl[i] = _mm512_permutex2var_epi16(t_1hh[2*i],mask_2l,t_1hh[2*i+1]);

        t_2hlh[i] = _mm512_permutex2var_epi16(t_1hl[2*i],mask_2h,t_1hl[2*i+1]);
        t_2hll[i] = _mm512_permutex2var_epi16(t_1hl[2*i],mask_2l,t_1hl[2*i+1]);

        t_2lhh[i] = _mm512_permutex2var_epi16(t_1lh[2*i],mask_2h,t_1lh[2*i+1]);
        t_2lhl[i] = _mm512_permutex2var_epi16(t_1lh[2*i],mask_2l,t_1lh[2*i+1]);

        t_2llh[i] = _mm512_permutex2var_epi16(t_1ll[2*i],mask_2h,t_1ll[2*i+1]);
        t_2lll[i] = _mm512_permutex2var_epi16(t_1ll[2*i],mask_2l,t_1ll[2*i+1]);
    }

    //giro con maschere 3
    for(int i = 0; i < 2; i++){
        t_3hhhh[i] = _mm512_permutex2var_epi16(t_2hhh[2*i],mask_3h,t_2hhh[2*i+1]);
        t_3hhhl[i] = _mm512_permutex2var_epi16(t_2hhh[2*i],mask_3l,t_2hhh[2*i+1]);

        t_3hhlh[i] = _mm512_permutex2var_epi16(t_2hhl[2*i],mask_3h,t_2hhl[2*i+1]);
        t_3hhll[i] = _mm512_permutex2var_epi16(t_2hhl[2*i],mask_3l,t_2hhl[2*i+1]);

        t_3hlhh[i] = _mm512_permutex2var_epi16(t_2hlh[2*i],mask_3h,t_2hlh[2*i+1]);
        t_3hlhl[i] = _mm512_permutex2var_epi16(t_2hlh[2*i],mask_3l,t_2hlh[2*i+1]);

        t_3hllh[i] = _mm512_permutex2var_epi16(t_2hll[2*i],mask_3h,t_2hll[2*i+1]);
        t_3hlll[i] = _mm512_permutex2var_epi16(t_2hll[2*i],mask_3l,t_2hll[2*i+1]);

        t_3lhhh[i] = _mm512_permutex2var_epi16(t_2lhh[2*i],mask_3h,t_2lhh[2*i+1]);
        t_3lhhl[i] = _mm512_permutex2var_epi16(t_2lhh[2*i],mask_3l,t_2lhh[2*i+1]);
        
        t_3lhlh[i] = _mm512_permutex2var_epi16(t_2lhl[2*i],mask_3h,t_2lhl[2*i+1]);
        t_3lhll[i] = _mm512_permutex2var_epi16(t_2lhl[2*i],mask_3l,t_2lhl[2*i+1]);
        
        t_3llhh[i] = _mm512_permutex2var_epi16(t_2llh[2*i],mask_3h,t_2llh[2*i+1]);
        t_3llhl[i] = _mm512_permutex2var_epi16(t_2llh[2*i],mask_3l,t_2llh[2*i+1]);

        t_3lllh[i] = _mm512_permutex2var_epi16(t_2lll[2*i],mask_3h,t_2lll[2*i+1]);
        t_3llll[i] = _mm512_permutex2var_epi16(t_2lll[2*i],mask_3l,t_2lll[2*i+1]);
        
    }

    block_out[0] = _mm512_permutex2var_epi16(t_3llll[0],mask_4l,t_3llll[1]);
    block_out[1] = _mm512_permutex2var_epi16(t_3llll[0],mask_4h,t_3llll[1]);

    block_out[2] = _mm512_permutex2var_epi16(t_3lllh[0],mask_4l,t_3lllh[1]);
    block_out[3] = _mm512_permutex2var_epi16(t_3lllh[0],mask_4h,t_3lllh[1]);

    block_out[4] = _mm512_permutex2var_epi16(t_3llhl[0],mask_4l,t_3llhl[1]);
    block_out[5] = _mm512_permutex2var_epi16(t_3llhl[0],mask_4h,t_3llhl[1]);

    block_out[6] = _mm512_permutex2var_epi16(t_3llhh[0],mask_4l,t_3llhh[1]);
    block_out[7] = _mm512_permutex2var_epi16(t_3llhh[0],mask_4h,t_3llhh[1]);

    block_out[8] = _mm512_permutex2var_epi16(t_3lhll[0],mask_4l,t_3lhll[1]);
    block_out[9] = _mm512_permutex2var_epi16(t_3lhll[0],mask_4h,t_3lhll[1]);

    block_out[10] = _mm512_permutex2var_epi16(t_3lhlh[0],mask_4l,t_3lhlh[1]);
    block_out[11] = _mm512_permutex2var_epi16(t_3lhlh[0],mask_4h,t_3lhlh[1]);

    block_out[12] = _mm512_permutex2var_epi16(t_3lhhl[0],mask_4l,t_3lhhl[1]);
    block_out[13] = _mm512_permutex2var_epi16(t_3lhhl[0],mask_4h,t_3lhhl[1]);

    block_out[14] = _mm512_permutex2var_epi16(t_3lhhh[0],mask_4l,t_3lhhh[1]);
    block_out[15] = _mm512_permutex2var_epi16(t_3lhhh[0],mask_4h,t_3lhhh[1]);

    block_out[16] = _mm512_permutex2var_epi16(t_3hlll[0],mask_4l,t_3hlll[1]);
    block_out[17] = _mm512_permutex2var_epi16(t_3hlll[0],mask_4h,t_3hlll[1]);

    block_out[18] = _mm512_permutex2var_epi16(t_3hllh[0],mask_4l,t_3hllh[1]);
    block_out[19] = _mm512_permutex2var_epi16(t_3hllh[0],mask_4h,t_3hllh[1]);

    block_out[20] = _mm512_permutex2var_epi16(t_3hlhl[0],mask_4l,t_3hlhl[1]);
    block_out[21] = _mm512_permutex2var_epi16(t_3hlhl[0],mask_4h,t_3hlhl[1]);

    block_out[22] = _mm512_permutex2var_epi16(t_3hlhh[0],mask_4l,t_3hlhh[1]);
    block_out[23] = _mm512_permutex2var_epi16(t_3hlhh[0],mask_4h,t_3hlhh[1]);

    block_out[24] = _mm512_permutex2var_epi16(t_3hhll[0],mask_4l,t_3hhll[1]);
    block_out[25] = _mm512_permutex2var_epi16(t_3hhll[0],mask_4h,t_3hhll[1]);

    block_out[26] = _mm512_permutex2var_epi16(t_3hhlh[0],mask_4l,t_3hhlh[1]);
    block_out[27] = _mm512_permutex2var_epi16(t_3hhlh[0],mask_4h,t_3hhlh[1]);

    block_out[28] = _mm512_permutex2var_epi16(t_3hhhl[0],mask_4l,t_3hhhl[1]);
    block_out[29] = _mm512_permutex2var_epi16(t_3hhhl[0],mask_4h,t_3hhhl[1]);

    block_out[30] = _mm512_permutex2var_epi16(t_3hhhh[0],mask_4l,t_3hhhh[1]);
    block_out[31] = _mm512_permutex2var_epi16(t_3hhhh[0],mask_4h,t_3hhhh[1]);
    
}