#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

#define PAZIENTI 20
#define DOTTORI 4

typedef struct{
    int rosso;
    int giallo;
    int verde;
    int bianco;
    pthread_mutex_t m;
} Ambulatorio;

Ambulatorio ambulatorio_adulti;
Ambulatorio ambulatorio_bambini;

void* accettazione();
void* cura_bambini();
void* cura_adulti();

int main(){
    pthread_t triage;
    pthread_t pediatri[DOTTORI];
    pthread_t dottori[DOTTORI];
    int rc;

    ambulatorio_adulti.rosso = 0;
    ambulatorio_adulti.giallo = 0;
    ambulatorio_adulti.verde = 0;
    ambulatorio_adulti.bianco = 0;

    rc = pthread_mutex_init(&ambulatorio_adulti.m,NULL);

    ambulatorio_bambini.rosso = 0;
    ambulatorio_bambini.giallo = 0;
    ambulatorio_bambini.verde = 0;
    ambulatorio_bambini.bianco = 0;

    rc = pthread_mutex_init(&ambulatorio_bambini.m,NULL);

    rc = pthread_create(&triage, NULL, accettazione, NULL);
    if(rc) {
        printf("Errore creazione thread triage %i\n",rc);
        exit(-1);
    }

    for(int i = 0; i < DOTTORI; i++){
        rc = pthread_create(&pediatri[i], NULL, cura_bambini, NULL);
        if(rc) {
            printf("Errore creazione thread pediatra %i: %i\n",i,rc);
            exit(-1);
        }

        rc = pthread_create(&dottori[i], NULL, cura_adulti, NULL);
        if(rc) {
            printf("Errore creazione thread pediatra %i: %i\n",i,rc);
            exit(-1);
        }
    }

    rc = pthread_join(triage,NULL);
    if(rc) {
        printf("Errore join thread triage: %i\n",rc);
        exit(-1);
    }


    for(int i = 0; i < DOTTORI; i++){
        rc = pthread_join(pediatri[i], NULL);
        if(rc) {
            printf("Errore join thread pediatra %i: %i\n",i,rc);
            exit(-1);
        }

        rc = pthread_join(dottori[i], NULL);
        if(rc) {
            printf("Errore join thread pediatra %i: %i\n",i,rc);
            exit(-1);
        }
    }

    printf("Sequenza terminata correttamente\n");
    
}

void* accettazione(){

    for(int i = 0; i < PAZIENTI; i++){
        int age = rand()%2; //0 bambino 1 adulto
        int codice = rand()%4; //0 rosso, 1 giallo, 2 verde, 3 bianco

        if(age){
            pthread_mutex_lock(&ambulatorio_adulti.m);
        } else {
            pthread_mutex_lock(&ambulatorio_bambini.m);
        }
    }
}