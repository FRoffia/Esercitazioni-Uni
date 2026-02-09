#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

#define MAX 3
#define UTENTI 10

typedef struct{
    int chiavi[UTENTI];// vettore con le chiavi degli utenti, 1 chiave presente, 0 consegnata, -1 restituita
    pthread_mutex_t m;
} Biglietteria;

typedef struct{
    int attesa_n;
    int attesa_s;
    int attesa_na;
    int attesa_sa;
    int dentro_n;
    int dentro_s;
    pthread_mutex_t m;
    pthread_cond_t LIBERA;
} Piscina;

Biglietteria big;
Piscina pis;

void* vai_in_piscina(void* arg);

int main(){
    srand(time(NULL));
    //===========================================================< Inizializzazione Variabili Memoria Condivisa >==============================================
    for(int i = 0; i < UTENTI; i++){big.chiavi[i] = 1;}
    pthread_mutex_init(&big.m,NULL);

    pis.attesa_n = 0;
    pis.attesa_s = 0;
    pis.attesa_na = 0;
    pis.attesa_sa = 0;
    pis.dentro_n = 0;
    pis.dentro_s = 0;
    pthread_mutex_init(&pis.m,NULL);
    pthread_cond_init(&pis.LIBERA,NULL);



    //===========================================================< Creazione Threads Utenti >==============================================
    pthread_t utenti[UTENTI];
    int rc = 0;

    for(int i = 0; i < UTENTI; i++){
        rc = pthread_create(&utenti[i],NULL,vai_in_piscina,i);
        if(rc){
            printf("Errore creazione thread utente %i. Errore: %i",i,rc);
            exit(-1);
        }
    }



    //===========================================================< Join Threads Utenti >==============================================
    for(int i = 0; i < UTENTI; i++){
        rc = pthread_join(utenti[i],NULL);
        if(rc){
            printf("Errore join thread utente %i. Errore: %i",i,rc);
            exit(-1);
        }
    }

    printf("Threads terminati correttamente.\n");

    for(int i = 0 ; i < UTENTI; i++){
        if(big.chiavi[i] != -1){
            printf("Chiave %i non restituita correttamente!!!!!");
            exit(-1);
        }
    }

    printf("Chiavi restituite correttamente\n");
}

void* vai_in_piscina(void* arg){
    //===========================================================< Inizializzazione utente >==============================================
    int id = (int) arg;
    int s_n = rand()%2;//1 studente 0 utente normale
    int a = rand()%2;//1 abbonato 0 no



    //===========================================================< Accesso biglietteria per chiavi >==============================================
    pthread_mutex_lock(&big.m);
    big.chiavi[id] = 0;//mi prendo le chiavi
    printf("Utente %i preso biglietto e chiavi dalla biglietteria.\n\n",id);
    pthread_mutex_unlock(&big.m);



    //===========================================================< Accesso piscina con priorità >==============================================
    pthread_mutex_lock(&pis.m);
    if(s_n){//studente

        if(a){//studente abbonato
            while(pis.dentro_n+pis.dentro_s == MAX || (pis.dentro_s > pis.dentro_n && pis.attesa_n+pis.attesa_na)){//aspetto finchè vedo che la piscina è piena o ci sono più studenti con utenti normali  in attesa
                pis.attesa_sa ++;
                pthread_cond_wait(&pis.LIBERA,&pis.m);
                pis.attesa_sa --;
            }
            pis.dentro_s ++;
            printf("Studente abbonato %i entrato in piscina.\nAttesa s_a:%i\nAttesa s:%i\nAttesa n_a:%i\nAttesa n:%i\nDentro s:%i\nDentro n:%i\n\n",id,pis.attesa_sa,pis.attesa_s,pis.attesa_na,pis.attesa_n,pis.dentro_s,pis.dentro_n);

        } else {//studente non abbonato
            while(pis.dentro_n+pis.dentro_s == MAX || (pis.dentro_s > pis.dentro_n && pis.attesa_n+pis.attesa_na) || pis.attesa_sa){//aspetto finchè vedo che la piscina è piena, ci sono più studenti con utenti normali in attesa o vedo che ci sono studenti abbonati in attesa
                pis.attesa_s ++;
                pthread_cond_wait(&pis.LIBERA,&pis.m);
                pis.attesa_s --;
            }
            pis.dentro_s ++;
            printf("Studente %i entrato in piscina.\nAttesa s_a:%i\nAttesa s:%i\nAttesa n_a:%i\nAttesa n:%i\nDentro s:%i\nDentro n:%i\n\n",id,pis.attesa_sa,pis.attesa_s,pis.attesa_na,pis.attesa_n,pis.dentro_s,pis.dentro_n);

        }

    } else {//utente normale

        if(a){//normale abbonato
            while(pis.dentro_n+pis.dentro_s == MAX || (pis.dentro_n >= pis.dentro_s && pis.attesa_s+pis.attesa_sa)){//aspetto finchè vedo che la piscina è piena o ci sono più normali con studenti in attesa
                pis.attesa_na ++;
                pthread_cond_wait(&pis.LIBERA,&pis.m);
                pis.attesa_na --;
            }
            pis.dentro_n ++;
            printf("Normale abbonato %i entrato in piscina.\nAttesa s_a:%i\nAttesa s:%i\nAttesa n_a:%i\nAttesa n:%i\nDentro s:%i\nDentro n:%i\n\n",id,pis.attesa_sa,pis.attesa_s,pis.attesa_na,pis.attesa_n,pis.dentro_s,pis.dentro_n);

        } else {//normale non abbonato
            while(pis.dentro_n+pis.dentro_s == MAX || (pis.dentro_n >= pis.dentro_s && pis.attesa_s+pis.attesa_sa) || pis.attesa_na){//aspetto finchè vedo che la piscina è piena, ci sono più normali con studenti in attesa o vedo che ci sono normali abbonati in attesa
                pis.attesa_n ++;
                pthread_cond_wait(&pis.LIBERA,&pis.m);
                pis.attesa_n --;
            }
            pis.dentro_n ++;
            printf("Normale %i entrato in piscina.\nAttesa s_a:%i\nAttesa s:%i\nAttesa n_a:%i\nAttesa n:%i\nDentro s:%i\nDentro n:%i\n\n",id,pis.attesa_sa,pis.attesa_s,pis.attesa_na,pis.attesa_n,pis.dentro_s,pis.dentro_n);

        }
    }
    pthread_mutex_unlock(&pis.m);




    //===========================================================< Esco dalla piscina >==============================================
    sleep(rand()%15);
    pthread_mutex_lock(&pis.m);
    if(s_n){
        pis.dentro_s --;
        printf("Studente %i uscito dalla piscina.\nAttesa s_a:%i\nAttesa s:%i\nAttesa n_a:%i\nAttesa n:%i\nDentro s:%i\nDentro n:%i\n\n",id,pis.attesa_sa,pis.attesa_s,pis.attesa_na,pis.attesa_n,pis.dentro_s,pis.dentro_n);
    } else {
        pis.dentro_n --;
        printf("Normale %i uscito dalla piscina.\nAttesa s_a:%i\nAttesa s:%i\nAttesa n_a:%i\nAttesa n:%i\nDentro s:%i\nDentro n:%i\n\n",id,pis.attesa_sa,pis.attesa_s,pis.attesa_na,pis.attesa_n,pis.dentro_s,pis.dentro_n);
    }
    pthread_cond_broadcast(&pis.LIBERA);
    pthread_mutex_unlock(&pis.m);



    //===========================================================< Restituisco chiavi e a lett >==============================================
    pthread_mutex_lock(&big.m);
    big.chiavi[id] = -1;
    printf("Utente %i restituito chiavi alla biglietteria.\n\n",id);
    pthread_mutex_unlock(&big.m);

    pthread_exit(NULL);
}