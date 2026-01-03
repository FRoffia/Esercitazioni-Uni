#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

#define N 20
#define MAX 4

typedef struct{
    int corsia_NORD;
    int corsia_SUD;
    bool corsia_invertita;
    pthread_mutex_t m;
} Strada;

Strada strada;

void *Entrata(int t_id){   //codice worker
    bool direzione; //0 sn, 1 ns
    bool dentro = 0;
    bool corsia; //0 nord, 1 sud

    direzione = (rand()%100 < 75) ? 1 : 0; //capisco se sta entrando da sud o da nord

    while(!dentro){
        pthread_mutex_lock(&strada.m);
        if(direzione){//caso ns

            if(strada.corsia_SUD < MAX){ //caso base corsia sud
                strada.corsia_SUD ++; //entro
                dentro = 1;
                corsia = 1;
                printf("Auto %i entrata da nord nella corsia sud\nSUD: %i\nNORD: %i\n\n\n", t_id,strada.corsia_SUD,strada.corsia_NORD);
                pthread_mutex_unlock(&strada.m);
                continue;

            } else if(strada.corsia_invertita){ //caso corsia invertita

                if(strada.corsia_NORD <= 0 && strada.corsia_NORD > MAX*(-1)){ //nessuna macchina da nord, si può entrare
                    strada.corsia_NORD --;
                    dentro = 1;
                    corsia = 0;
                    printf("Auto %i entrata da nord nella corsia nord\nSUD: %i\nNORD: %i\n\n\n", t_id,strada.corsia_SUD,strada.corsia_NORD);
                    pthread_mutex_unlock(&strada.m);
                    continue;

                } else { //la corsia è invertita ma siamo pieni o stiamo aspettando che si liberi lo spazio
                    pthread_mutex_unlock(&strada.m);
                    sleep(1);
                    continue;

                }

            } else { //caso corsia sud piena, bisogna invertire la corsia nord per dare priorità a chi viene da nord
                strada.corsia_invertita = 1;

                if(strada.corsia_NORD <= 0 && strada.corsia_NORD > MAX*(-1)){//possiamo entrare subito
                    strada.corsia_NORD --;
                    dentro = 1;
                    corsia = 0;
                    printf("Auto %i entrata da nord nella corsia nord\nSUD: %i\nNORD: %i\n\n\n", t_id,strada.corsia_SUD,strada.corsia_NORD);
                    pthread_mutex_unlock(&strada.m);
                    continue;

                } else {//bisogna aspettare che la corsia nord si liberi da chi proveniva da sud
                    pthread_mutex_unlock(&strada.m);
                    sleep(1);
                    continue;

                }
            }

        } else {//caso sn

            if(!strada.corsia_invertita && strada.corsia_NORD < MAX && strada.corsia_NORD >= 0){//caso corsia NORD con senso di marcia normale, capienza minore del limite e nessuna macchina proveniente da nord
                strada.corsia_NORD ++;
                dentro = 1;
                corsia = 0;
                printf("Auto %i entrata da sud nella corsia nord\nSUD: %i\nNORD: %i\n\n\n", t_id,strada.corsia_SUD,strada.corsia_NORD);
                pthread_mutex_unlock(&strada.m);
                continue;

            } else {//caso strada invertita, bisogna aspettare che venga liberata la corsia SUD e quindi invertito di nuovo il senso di marcia, e che le auto provenienti da nord liberino la corsia
                pthread_mutex_unlock(&strada.m);
                sleep(1);
                continue;

            }
        }
    }

    //a questo punto siamo entrati nella strada, aspettiamo un tempo random e ricominciamo

    sleep(rand()%5+1);

    pthread_mutex_lock(&strada.m);

    if(direzione){ //caso ns
        
        if(corsia){//caso entrato dalla corsia SUD
            strada.corsia_SUD--;
            if(strada.corsia_invertita) strada.corsia_invertita = 0;//inverto di nuovo il senso di marcia siccome ho spazio libero nella corsia SUD
            printf("Auto %i uscita a sud dalla corsia sud\nSUD: %i\nNORD: %i\n\n\n", t_id,strada.corsia_SUD,strada.corsia_NORD);

        } else {//caso entrato dalla corsia NORD
            strada.corsia_NORD++;
            printf("Auto %i uscita a sud dalla corsia nord\nSUD: %i\nNORD: %i\n\n\n", t_id,strada.corsia_SUD,strada.corsia_NORD);
            
        }

    } else { //caso sn
        strada.corsia_NORD--;
        printf("Auto %i uscita a nord dalla corsia nord\nSUD: %i\nNORD: %i\n\n\n", t_id,strada.corsia_SUD,strada.corsia_NORD);

    }

    pthread_mutex_unlock(&strada.m);

    pthread_exit(NULL);
}


int main (int argc, char *argv[]){   
    pthread_t thread[N];
    int rc;

    strada.corsia_NORD = 0;
    strada.corsia_SUD = 0;
    strada.corsia_invertita = 0;
    rc = pthread_mutex_init(&strada.m,NULL);

    srand(time(NULL));

    for(int i = 0 ; i < N ; i++){
        printf("Main: creazione thread n.%d\n", i);
        rc = pthread_create(&thread[i], NULL, Entrata, i);
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

    printf("Sequenza terminata.\n");
    
}
