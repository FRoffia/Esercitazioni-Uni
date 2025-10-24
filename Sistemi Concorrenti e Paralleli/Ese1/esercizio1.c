#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#define N 2000
#define K 40

typedef struct{
    int voti[K];
    int pareri;
    pthread_mutex_t m;
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
    rc = pthread_mutex_init(&sondaggio.m,NULL);

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
		    printf("ERRORE join thread %ld codice %d\n", i, rc);
    }

    for(int i = 0; i < K; i++){
        printf("main-risultato finale per film %d: %.2f\n",i, (float)sondaggio.voti[i]/sondaggio.pareri);
    }
    

}
