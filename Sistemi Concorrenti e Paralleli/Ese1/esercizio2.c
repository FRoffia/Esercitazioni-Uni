#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <semaphore.h>


#define N 20
#define K 40

typedef struct{
    int voti[K];
    int pareri;
    pthread_mutex_t m;
    int sondaggiCompletati;
    sem_t s;
} Sondaggio;

Sondaggio sondaggio;

void *Votazione(){   //codice worker
    pthread_mutex_lock(&sondaggio.m);

    for(int i =0; i < K; i++){
        sondaggio.voti[i] += 1+(rand()%10);
    }

    sondaggio.pareri++;

    printf("Finito thread, votazioni attuali:\n");
    for(int i = 0; i < K ; i++){
        printf("risultato intermedio per film%d: %.2f\n", i, (float)sondaggio.voti[i]/sondaggio.pareri);
    }

    sondaggio.sondaggiCompletati++;

    //l'ultimo thread che finisce sveglia gli altri
    if(sondaggio.sondaggiCompletati == N){
        for(int i = 0; i < N-1; i++){
            sem_post(&sondaggio.s);
        }
    }

    pthread_mutex_unlock(&sondaggio.m);


    //fase di download e di visione
    pthread_mutex_lock(&sondaggio.m);

    while(sondaggio.sondaggiCompletati < N){//teoricamente basterebbe anche un if visto che si tratta di barriera ma va bene così
        pthread_mutex_unlock(&sondaggio.m);
        printf("Attesa fine sondaggi....\n");
        sem_wait(&sondaggio.s);
        printf("Sondaggi completati, si riparte\n");
        pthread_mutex_lock(&sondaggio.m);
    }

    sleep(rand()%10);//Download
    printf("Download terminato, rilascio mutex\n");

    pthread_mutex_unlock(&sondaggio.m);

    pthread_exit(NULL);
}


int main (int argc, char *argv[]){   
    pthread_t thread[N];
    int rc;

    for(int i = 0 ; i < K ; i++){
        sondaggio.voti[i] = 0;
    }
    sondaggio.pareri = 0;
    sondaggio.sondaggiCompletati = 0;
    rc = pthread_mutex_init(&sondaggio.m,NULL);
    rc = sem_init(&sondaggio.s, 1,0);

    srand(time(NULL));

    

    for(int i = 0 ; i < N ; i++){
        printf("Main: creazione thread n.%d\n", i);
        rc = pthread_create(&thread[i], NULL, Votazione, NULL);
        if (rc) {
            printf("ERRORE: %d\n", rc);
            exit(-1);   
        }
    }

    for(int i=0; i<N; i++) {
        rc = pthread_join(thread[i], NULL);
	    if (rc)
		    printf("ERRORE join thread %d codice %d\n", i, rc);
    }

    for(int i = 0; i < K; i++){
        printf("main-risultato finale per film %d: %.2f\n",i, (float)sondaggio.voti[i]/sondaggio.pareri);
    }


    

}
