#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <semaphore.h>

#define MAX_B  5
#define MAX_M  5
#define N  20

typedef struct{
    int posti_liberi;
    int bici_libere;
    int monopattini_liberi;
    sem_t s;
    int thread_sospesi;
    pthread_mutex_t m;
} Biglietteria;

Biglietteria biglietteria;

void* visita (int t_id){
    pthread_mutex_lock(&biglietteria.m);
    int biciMono = rand()%2;

    if(biciMono){
        //bici
        while(biglietteria.posti_liberi == 0 || biglietteria.bici_libere == 0){
            printf("Thread %i in attesa\n",t_id);
            biglietteria.thread_sospesi ++;
            pthread_mutex_unlock(&biglietteria.m);
            printf("Thread %i in pista\n",t_id);
            sem_wait(&biglietteria.s);
            pthread_mutex_lock(&biglietteria.m);
            biglietteria.thread_sospesi --;
        }

        

        biglietteria.bici_libere --;
        biglietteria.posti_liberi --;

        printf("Thread %i entrato. Bici rimanenti: %i, Monopattini rimanenti %i, Posti rimanenti : %i\n",t_id,biglietteria.bici_libere,biglietteria.monopattini_liberi,biglietteria.posti_liberi);

        pthread_mutex_unlock(&biglietteria.m);

    } else {
        //monop
        while(biglietteria.posti_liberi == 0 || biglietteria.monopattini_liberi == 0){
            printf("Thread %i in attesa\n",t_id);
            biglietteria.thread_sospesi ++;
            pthread_mutex_unlock(&biglietteria.m);
            sem_wait(&biglietteria.s);
            printf("Thread %i in pista\n",t_id);
            pthread_mutex_lock(&biglietteria.m);
            biglietteria.thread_sospesi --;
        }

        


        biglietteria.monopattini_liberi --;
        biglietteria.posti_liberi --;

        printf("Thread %i entrato. Bici rimanenti: %i, Monopattini rimanenti %i, Posti rimanenti : %i\n",t_id,biglietteria.bici_libere,biglietteria.monopattini_liberi,biglietteria.posti_liberi);

        pthread_mutex_unlock(&biglietteria.m);
    }

    sleep(rand()%10);

    pthread_mutex_lock(&biglietteria.m);

    

    biglietteria.posti_liberi ++;

    if(biciMono){
        biglietteria.bici_libere ++;
    } else {
        biglietteria.monopattini_liberi ++;
    }

    for(int i = 0; i < biglietteria.thread_sospesi; i++){
        sem_post(&biglietteria.s);
    }


    printf("Thread %i fuori. Bici rimanenti: %i, Monopattini rimanenti %i, Posti rimanenti : %i\n",t_id,biglietteria.bici_libere,biglietteria.monopattini_liberi,biglietteria.posti_liberi,t_id);

    pthread_mutex_unlock(&biglietteria.m);

    pthread_exit(NULL);
}


int main (int argc, char *argv[]){   
    pthread_t thread[N];
    int rc;

    biglietteria.bici_libere = MAX_B;
    biglietteria.monopattini_liberi = MAX_M;
    biglietteria.posti_liberi = MAX_B+MAX_M;
    biglietteria.thread_sospesi = 0;   
    rc = pthread_mutex_init(&biglietteria.m,NULL);
    rc = sem_init(&biglietteria.s, 1,0);

    srand(time(NULL));

    

    for(int i = 0 ; i < N ; i++){
        printf("Main: creazione thread n.%d\n", i);
        rc = pthread_create(&thread[i], NULL, visita, i);
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
    

}
