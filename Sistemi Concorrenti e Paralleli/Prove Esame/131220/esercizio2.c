#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

#define MAX 200
#define N 100
#define ATTIVAZIONI_RUBINETTI 10

typedef struct {
    int acqua;
    pthread_mutex_t m;
    pthread_cond_t CISTERNA_VUOTA;
    pthread_cond_t CISTERNA_RIEMPITA;
} Cisterna;

Cisterna cisterna;

void* riempi();
void* svuota();

int main(){
    pthread_t autocisterna;
    pthread_t rubinetti[N];

    int rc = 0;

    cisterna.acqua = 0;
    pthread_mutex_init(&cisterna.m,NULL); 
    pthread_cond_init(&cisterna.CISTERNA_VUOTA,NULL); 
    pthread_cond_init(&cisterna.CISTERNA_RIEMPITA,NULL); 

    rc = pthread_create(&autocisterna,NULL,riempi,NULL);
    if(rc){
        printf("Errore creazione thread autocisterna, codice:%i",rc);
        exit(-1);
    }

    for(int i = 0; i < N; i++){
        rc = pthread_create(&rubinetti[i],NULL,svuota,NULL);
        if(rc){
            printf("Errore creazione thread rubinetto %i, codice:%i",i,rc);
            exit(-1);
        }
    }

    rc=pthread_join(autocisterna,NULL);
    if(rc){
        printf("Errore join thread autocisterna, codice:%i",rc);
        exit(-1);
    }

    for(int i = 0; i < N; i++){
        rc = pthread_join(rubinetti[i],NULL);
        if(rc){
            printf("Errore join thread rubinetto %i, codice:%i",i,rc);
            exit(-1);
        }
    }

    printf("Sessione terminata correttamente\n");
}

void* riempi(){
    
    for(int i = 0; i < N*ATTIVAZIONI_RUBINETTI/MAX; i++){
        pthread_mutex_lock(&cisterna.m);

        while(cisterna.acqua > 0){
            pthread_cond_wait(&cisterna.CISTERNA_VUOTA,&cisterna.m);
        }

        cisterna.acqua = MAX;
    }
}