#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <semaphore.h>

#define MAX_P  20//massimo persone nel parco
#define MAX_A  5//massimo auto elettriche, quindi max gruppi
#define N 20//gruppi che vogliono accedere

typedef struct{
    int posti_liberi;
    int auto_libere;
    int contatore_entrata;
    sem_t * ordine_entrata;
    int thread_sospesi;
    pthread_mutex_t m;
} Biglietteria;

Biglietteria biglietteria;

void* visita (int t_id){
    pthread_mutex_lock(&biglietteria.m);

    int persone = rand()%5+1;
    int postoInFila = biglietteria.contatore_entrata;
    
    //posto riservato

    printf("Thread/Gruppo %i accede in biglietteria con posto %i, %i persone. Auto rimanenti: %i, Posti rimanenti: %i\n",t_id,postoInFila,persone,biglietteria.auto_libere,biglietteria.posti_liberi);

    biglietteria.contatore_entrata++;
    //accesso al parco
    while(biglietteria.posti_liberi < persone || biglietteria.auto_libere == 0){
        printf("Thread/Gruppo %i in attesa\n",t_id);
        biglietteria.thread_sospesi ++;
        pthread_mutex_unlock(&biglietteria.m);
        sem_wait(&biglietteria.ordine_entrata[postoInFila]);
        printf("Thread %i in pista\n",t_id);
        pthread_mutex_lock(&biglietteria.m);
        biglietteria.thread_sospesi --;
    }

    biglietteria.auto_libere --;
    biglietteria.posti_liberi -= persone;

    printf("Thread %i entrato con posto %i, %i persone. Auto rimanenti: %i, Posti rimanenti: %i\n",t_id,postoInFila,persone,biglietteria.auto_libere,biglietteria.posti_liberi);

    pthread_mutex_unlock(&biglietteria.m);

    sleep(rand()%10);


    //libero il monopattino/bici
    pthread_mutex_lock(&biglietteria.m);

    biglietteria.posti_liberi +=persone;

    biglietteria.auto_libere ++;

    sem_post(&biglietteria.ordine_entrata[postoInFila]);

    printf("Thread %i fuori con %i persone. Auto rimanenti: %i, Posti rimanenti: %i\n",t_id,persone,biglietteria.auto_libere,biglietteria.posti_liberi);

    int valore_semaforo,prossimo = 0;
    sem_getvalue(&biglietteria.ordine_entrata[prossimo],&valore_semaforo);
    printf("Provo a svegliare posto %i, valore: %i\n",prossimo,valore_semaforo);
    while(valore_semaforo != 0 && prossimo < biglietteria.contatore_entrata) {
        prossimo++;
        sem_getvalue(&biglietteria.ordine_entrata[prossimo],&valore_semaforo);
        printf("Provo a svegliare posto %i, valore: %i\n",prossimo,valore_semaforo);
    }
    sem_post(&biglietteria.ordine_entrata[prossimo]);

    

    pthread_mutex_unlock(&biglietteria.m);

    pthread_exit(NULL);
}


int main (int argc, char *argv[]){   
    pthread_t thread[N];
    int rc;

    biglietteria.auto_libere = MAX_A;
    biglietteria.posti_liberi = MAX_P;
    biglietteria.thread_sospesi = 0;
    biglietteria.contatore_entrata = 0;
    biglietteria.ordine_entrata = (sem_t*) malloc(N*sizeof(sem_t));//lo creo di dimensione = N per caso peggiore in cui tutti i gruppi sono da una persona
    for(int i = 0; i < N; i++){
        rc = sem_init(&biglietteria.ordine_entrata[i], 1,0);
    }
    rc = pthread_mutex_init(&biglietteria.m,NULL);

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
