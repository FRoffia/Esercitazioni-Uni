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
    int done;
    char tipo[10];
    pthread_mutex_t m;
    pthread_cond_t NUOVO_PAZIENTE;
} Ambulatorio;

Ambulatorio ambulatorio_adulti;
Ambulatorio ambulatorio_bambini;

void* accettazione();
void* cura(void* arg);

int main(){
    pthread_t triage;
    pthread_t pediatri[DOTTORI];
    pthread_t dottori[DOTTORI];
    int rc;

    ambulatorio_adulti.rosso = 0;
    ambulatorio_adulti.giallo = 0;
    ambulatorio_adulti.verde = 0;
    ambulatorio_adulti.bianco = 0;
    ambulatorio_adulti.done = 0;
    strcpy(ambulatorio_adulti.tipo,"adulti");

    rc = pthread_mutex_init(&ambulatorio_adulti.m,NULL);
    rc = pthread_cond_init(&ambulatorio_adulti.NUOVO_PAZIENTE,NULL);

    ambulatorio_bambini.rosso = 0;
    ambulatorio_bambini.giallo = 0;
    ambulatorio_bambini.verde = 0;
    ambulatorio_bambini.bianco = 0;
    ambulatorio_bambini.done = 0;
    strcpy(ambulatorio_bambini.tipo,"bambini");

    rc = pthread_mutex_init(&ambulatorio_bambini.m,NULL);
    rc = pthread_cond_init(&ambulatorio_bambini.NUOVO_PAZIENTE,NULL);


    rc = pthread_create(&triage, NULL, accettazione, NULL);
    if(rc) {
        printf("Errore creazione thread triage %i\n",rc);
        exit(-1);
    }

    for(int i = 0; i < DOTTORI; i++){
        rc = pthread_create(&pediatri[i], NULL, cura, &ambulatorio_bambini);
        if(rc) {
            printf("Errore creazione thread pediatra %i: %i\n",i,rc);
            exit(-1);
        }

        rc = pthread_create(&dottori[i], NULL, cura, &ambulatorio_adulti);
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

    Ambulatorio* ambulatorio;

    for(int i = 0; i < PAZIENTI; i++){
        int age = rand()%2; //0 bambino 1 adulto
        int codice = rand()%4; //0 rosso, 1 giallo, 2 verde, 3 bianco

        ambulatorio = age ? &ambulatorio_adulti : &ambulatorio_bambini;

        pthread_mutex_lock(&ambulatorio->m);

        switch(codice){
            case 0:
                ambulatorio->rosso ++;
                printf("Inserito paziente rosso all'interno dell'ambulatorio %s\n",ambulatorio->tipo);
                break;

            case 1:
                ambulatorio->giallo ++;
                printf("Inserito paziente giallo all'interno dell'ambulatorio %s\n",ambulatorio->tipo);

                break;

            case 2:
                ambulatorio->verde ++;
                printf("Inserito paziente verde all'interno dell'ambulatorio %s\n",ambulatorio->tipo);
                break;

            case 3:
                ambulatorio->bianco ++;
                printf("Inserito paziente bianco all'interno dell'ambulatorio %s\n",ambulatorio->tipo);
                break;

            default:
                break;
        }
        sleep(rand()%2);
        pthread_cond_signal(&ambulatorio->NUOVO_PAZIENTE);
        pthread_mutex_unlock(&ambulatorio->m);

    }

    pthread_mutex_lock(&ambulatorio_adulti.m);
    ambulatorio_adulti.done = 1;
    pthread_cond_broadcast(&ambulatorio_adulti.NUOVO_PAZIENTE);
    pthread_mutex_unlock(&ambulatorio_adulti.m);

    pthread_mutex_lock(&ambulatorio_bambini.m);
    ambulatorio_bambini.done = 1;
    pthread_cond_broadcast(&ambulatorio_bambini.NUOVO_PAZIENTE);
    pthread_mutex_unlock(&ambulatorio_bambini.m);

    printf("!!!!!!!!!!FINITI PAZIENTI!!!!!!!!!!!!!!!!!\n\n");

    pthread_exit(NULL);
}

void* cura(void* arg){
    Ambulatorio * ambulatorio = (Ambulatorio*) arg;

    sleep(10);

    while(true){
    
        pthread_mutex_lock(&ambulatorio->m);

        //=======================================< Controllo condizioni di uscita e di attesa >======================================================
        if (ambulatorio->rosso == 0 && ambulatorio->giallo == 0 && ambulatorio->verde == 0 && ambulatorio->bianco == 0 && ambulatorio->done){//non ci sono più pazienti
            pthread_mutex_unlock(&ambulatorio->m);
            pthread_exit(NULL);
        } else {
            while(ambulatorio->rosso == 0 && ambulatorio->giallo == 0 && ambulatorio->verde == 0 && ambulatorio->bianco == 0 && !ambulatorio->done){//stanno ancora entrando pazienti ma in questo momento non ce ne sono
                pthread_cond_wait(&ambulatorio->NUOVO_PAZIENTE,&ambulatorio->m);
            }
        }
        

        //=======================================< Cura del paziente >======================================================
        int *codice_urgenza;
        char codice[10];

        if(ambulatorio->rosso > 0){
            codice_urgenza = &ambulatorio->rosso;
            strcpy(codice,"rosso");
        } else if(ambulatorio->giallo > 0){
            codice_urgenza = &ambulatorio->giallo;
            strcpy(codice,"giallo");
        } else if(ambulatorio->verde > 0){
            codice_urgenza = &ambulatorio->verde;
            strcpy(codice,"verde");
        } else if(ambulatorio->bianco > 0){
            codice_urgenza = &ambulatorio->bianco;
            strcpy(codice,"bianco");
        } else {
            sleep(rand()%3);
            pthread_mutex_unlock(&ambulatorio->m);
            continue;
        }

        (*codice_urgenza) --;
        printf("Curato paziente %s dell'ambulatorio %s\nrossi:%i\ngialli:%i\nverdi:%i\nbianchi:%i\n\n\n",codice, ambulatorio->tipo,ambulatorio->rosso,ambulatorio->giallo,ambulatorio->verde,ambulatorio->bianco);
        sleep(rand()%4);
        pthread_mutex_unlock(&ambulatorio->m);
        
    }
}