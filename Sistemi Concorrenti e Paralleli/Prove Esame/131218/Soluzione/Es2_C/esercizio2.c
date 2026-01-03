#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

#define N 100
#define MAX 10

typedef struct{
    int primi_piatti;
    int secondi_piatti;
    pthread_mutex_t m;
    pthread_cond_t TAVOLO_SVUOTATO;
    pthread_cond_t TAVOLO_RIEMPITO_PRIMO;
    pthread_cond_t TAVOLO_RIEMPITO_SECONDO;
} Tavolo;

Tavolo tavolo;

void* servi_piatti();
void* self_service(void* t_id);


int main(){


    //==================< Dichiarazione variabili e inizializzazioni >==================================================
    pthread_t dipendenti[N];
    pthread_t cuoco;
    int rc;

    tavolo.primi_piatti = 0;
    tavolo.secondi_piatti = 0;
    rc = pthread_mutex_init(&tavolo.m,NULL);
    pthread_cond_init(&tavolo.TAVOLO_SVUOTATO, NULL);
    pthread_cond_init(&tavolo.TAVOLO_RIEMPITO_PRIMO, NULL);
    pthread_cond_init(&tavolo.TAVOLO_RIEMPITO_SECONDO, NULL);


    srand(time(NULL));

    //==================< Creazione Thread Cuoco >==================================================
    printf("Main: creazione thread cuoco\n");
    rc = pthread_create(&cuoco,NULL,servi_piatti,NULL);
    if(rc){
        printf("Main: errore creazione thread cuoco: %d\n",rc);
        exit(-1);
    }

    //==================< Creazione Threads Dipendenti >==================================================
    for(int i = 0; i < N; i++){
        printf("Main: creazione thread %i\n", i+1);
        rc = pthread_create(&dipendenti[i],NULL,self_service,i+1);
        if(rc){
            printf("Main: errore creazione thread i:%i, rc:%i\n",i+1,rc);
            exit(-1);
        }
    }


    //==================< Join cuoco e dipendenti >==================================================
    rc = pthread_join(cuoco,NULL);
    if(rc){
        printf("Main: errore join del thread cuoco: %i\n",rc);
        exit(-1);
    }

    for(int i = 0; i < N ; i++){
        rc = pthread_join(dipendenti[i],NULL);
        if(rc){
            printf("Main: errore join thread dipendente %i, rc:%i\n",i,rc);
            exit(-1);
        }
    }

    printf("Sequenza terminata\n;");

}

void* servi_piatti(){

    //==================< Servizio primi piatti >==================================================
    for(int i = 0; i < N; i++){
        pthread_mutex_lock(&tavolo.m);

        if(tavolo.primi_piatti == MAX) pthread_cond_wait(&tavolo.TAVOLO_SVUOTATO,&tavolo.m);
        tavolo.primi_piatti ++;
        printf("Cuoco: primo %i servito.\nPrimi: %i\nSecondi: %i\n\n\n",i+1,tavolo.primi_piatti,tavolo.secondi_piatti);
        pthread_cond_signal(&tavolo.TAVOLO_RIEMPITO_PRIMO);
        pthread_mutex_unlock(&tavolo.m);

        //sleep(rand()%5+1);//tempo di preparazione del piatto
    }

    //==================< Servizio secondi piatti >==================================================
    for(int i = 0; i < N; i++){
        pthread_mutex_lock(&tavolo.m);

        while(tavolo.primi_piatti > 0 || tavolo.secondi_piatti == MAX) pthread_cond_wait(&tavolo.TAVOLO_SVUOTATO,&tavolo.m);
        tavolo.secondi_piatti ++;
        printf("Cuoco: secondo %i servito.\nPrimi: %i\nSecondi: %i\n\n\n",i+1,tavolo.primi_piatti,tavolo.secondi_piatti);
        pthread_cond_signal(&tavolo.TAVOLO_RIEMPITO_SECONDO);
        pthread_mutex_unlock(&tavolo.m);

        //sleep(rand()%5+1);//tempo di preparazione del piatto
    }
}


void* self_service(void* t_id){

    //==================< Primo >==================================================
    pthread_mutex_lock(&tavolo.m);
    while(tavolo.primi_piatti == 0) pthread_cond_wait(&tavolo.TAVOLO_RIEMPITO_PRIMO,&tavolo.m);
    tavolo.primi_piatti--;
    printf("Dipendente %i: primo mangiato.\nPrimi: %i\nSecondi: %i\n\n\n",t_id,tavolo.primi_piatti,tavolo.secondi_piatti);
    pthread_cond_signal(&tavolo.TAVOLO_SVUOTATO);
    pthread_mutex_unlock(&tavolo.m);

    sleep(rand()%5+1);

    //==================< Secondo >==================================================
    pthread_mutex_lock(&tavolo.m);
    while(tavolo.secondi_piatti == 0) pthread_cond_wait(&tavolo.TAVOLO_RIEMPITO_SECONDO,&tavolo.m);
    tavolo.secondi_piatti--;
    printf("Dipendente %i: secondo mangiato.\nPrimi: %i\nSecondi: %i\n\n\n",t_id,tavolo.primi_piatti,tavolo.secondi_piatti);
    pthread_cond_signal(&tavolo.TAVOLO_SVUOTATO);
    pthread_mutex_unlock(&tavolo.m);

}