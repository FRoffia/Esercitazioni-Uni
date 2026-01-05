#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define N 10 //numero sportelli
#define CLIENTI 100

typedef struct{
    int occupati;
    int attesa_over;
    int attesa_under;
    pthread_mutex_t m;
    pthread_cond_t SPORTELLO_LIBERATO;
}Sportelli;

typedef struct{
    int occupato;
    int attesa_over;
    int attesa_under;
    int serviti_over;
    int serviti_under;
    pthread_mutex_t m;
    pthread_cond_t CAVEAU_LIBERATO;
}Caveau;

Sportelli sportelli;
Caveau caveau;

void* accesso_banca(void* arg);

int main(){
    //=========================< Dichiarazione threads e inizializzazione risorse condivise>===============================================================
    pthread_t clienti[CLIENTI];
    int rc;

    sportelli.occupati = 0;
    sportelli.attesa_over = 0;
    sportelli.attesa_under = 0;
    pthread_mutex_init(&sportelli.m,NULL);
    pthread_cond_init(&sportelli.SPORTELLO_LIBERATO,NULL);

    caveau.occupato = 0;
    caveau.attesa_over = 0;
    caveau.attesa_under = 0;
    caveau.serviti_over = 0;
    caveau.serviti_under = 0;
    pthread_mutex_init(&caveau.m,NULL);
    pthread_cond_init(&caveau.CAVEAU_LIBERATO,NULL);



    //=========================< Creazione threads clienti >===============================================================
    for(int i = 0; i < CLIENTI; i++){
        rc = pthread_create(&clienti[i],NULL,accesso_banca,i);
        if(rc){
            printf("Errore creazione thread cliente %i, codice:%i\n",i,rc);
            exit(-1);
        }
    }



    //=========================< Join threads clienti >===============================================================
    for(int i = 0; i < CLIENTI; i++){
        rc = pthread_join(clienti[i],NULL);
        if(rc){
            printf("Errore join thread cliente %i, codice:%i\n",i,rc);
            exit(-1);
        }
    }

    printf("Threads terminati correttamente, sequenza terminata.\n");
}






void* accesso_banca(void* arg){
    int t_id = (int)arg;
    int eta = rand()%82+18;

    //=========================< Attesa per accesso agli sportelli con priorità >===============================================================
    pthread_mutex_lock(&sportelli.m);
    if(eta < 70){//se under aspetto sia che uno sporello sia libero sia che non ci siano over 70 in attesa 
        while(sportelli.occupati == N || sportelli.attesa_over > 0){
            sportelli.attesa_under ++;
            pthread_cond_wait(&sportelli.SPORTELLO_LIBERATO,&sportelli.m);
            sportelli.attesa_under --;
        }
    } else {//se over semplicemente aspetto che si liberi uno sportello
        while(sportelli.occupati == N){
            sportelli.attesa_over ++;
            pthread_cond_wait(&sportelli.SPORTELLO_LIBERATO,&sportelli.m);
            sportelli.attesa_over --;
        }
    }
    sportelli.occupati ++;
    printf("Cliente %i di eta %i entrato negli sportelli.\nOccupati:%i\nAttesa_over:%i\nAttesa_under:%i\n\n",t_id,eta,sportelli.occupati,sportelli.attesa_over,sportelli.attesa_under);
    pthread_mutex_unlock(&sportelli.m);



    //=========================< Ottenimento chiave e svuotamento sportello >===============================================================
    sleep(3);//simulazione ottenimento chiave da parte del commesso
    pthread_mutex_lock(&sportelli.m);
    sportelli.occupati --;
    printf("Cliente %i di eta %i uscito dagli sportelli, verso caveau.\nOccupati:%i\nAttesa_over:%i\nAttesa_under:%i\n\n",t_id,eta,sportelli.occupati,sportelli.attesa_over,sportelli.attesa_under);
    pthread_cond_broadcast(&sportelli.SPORTELLO_LIBERATO);
    pthread_mutex_unlock(&sportelli.m);



    //=========================< Attesa accesso a caveau con priorità >===============================================================
    pthread_mutex_lock(&caveau.m);
    if(eta < 70){
        while(caveau.occupato || (caveau.serviti_over < caveau.serviti_under && caveau.attesa_over > 0)){//cotrollo che il caveau non sia occupato e che venga rispettata la precedenza di accesso per la categoria meno servita storicamente
            caveau.attesa_under ++;
            pthread_cond_wait(&caveau.CAVEAU_LIBERATO,&caveau.m);
            caveau.attesa_under --;
        }
    } else {
        while(caveau.occupato || (caveau.serviti_under < caveau.serviti_over && caveau.attesa_under > 0)){//cotrollo che il caveau non sia occupato e che venga rispettata la precedenza di accesso per la categoria meno servita storicamente
            caveau.attesa_over ++;
            pthread_cond_wait(&caveau.CAVEAU_LIBERATO,&caveau.m);
            caveau.attesa_over --;
        }
    }
    caveau.occupato = 1;
    printf("Cliente %i di eta %i entrato nel caveau.\nOccupato:%i\nAttesa_over:%i\nAttesa_under:%i\nServiti_over:%i\nServiti_under:%i\n\n",t_id,eta,caveau.occupato,caveau.attesa_over,caveau.attesa_under,caveau.serviti_over,caveau.serviti_under);
    pthread_mutex_unlock(&caveau.m);



    //=========================< Controllo cassetta e uscita dal caveau >===============================================================
    sleep(1);//simulazione controllo cassetta di sicurezza
    pthread_mutex_lock(&caveau.m);
    caveau.occupato = 0;
    if(eta < 70) caveau.serviti_under ++;
    else caveau.serviti_over ++;
    printf("Cliente %i di eta %i uscito dal caveau.\nOccupato:%i\nAttesa_over:%i\nAttesa_under:%i\nServiti_over:%i\nServiti_under:%i\n\n",t_id,eta,caveau.occupato,caveau.attesa_over,caveau.attesa_under,caveau.serviti_over,caveau.serviti_under);
    pthread_cond_broadcast(&caveau.CAVEAU_LIBERATO);
    pthread_mutex_unlock(&caveau.m);



    //=========================< Attesa accesso agli sportelli con priorità per restituire chiave >===============================================================
    pthread_mutex_lock(&sportelli.m);
    if(eta < 70){//se under aspetto sia che uno sporello sia libero sia che non ci siano over 70 in attesa 
        while(sportelli.occupati == N || sportelli.attesa_over > 0){
            sportelli.attesa_under ++;
            pthread_cond_wait(&sportelli.SPORTELLO_LIBERATO,&sportelli.m);
            sportelli.attesa_under --;
        }
    } else {//se over semplicemente aspetto che si liberi uno sportello
        while(sportelli.occupati == N){
            sportelli.attesa_over ++;
            pthread_cond_wait(&sportelli.SPORTELLO_LIBERATO,&sportelli.m);
            sportelli.attesa_over --;
        }
    }
    sportelli.occupati ++;
    printf("Cliente %i di eta %i entrato negli sportelli per restituire la chiave.\nOccupati:%i\nAttesa_over:%i\nAttesa_under:%i\n\n",t_id,eta,sportelli.occupati,sportelli.attesa_over,sportelli.attesa_under);
    pthread_mutex_unlock(&sportelli.m);

    

    //=========================< Restituzione chiave e si va a letto >===============================================================
    sleep(1);//simulazione restituzione chiave
    pthread_mutex_lock(&sportelli.m);
    sportelli.occupati --;
    printf("!Terminato! Cliente %i di eta %i uscito dagli sportelli, verso casa.\nOccupati:%i\nAttesa_over:%i\nAttesa_under:%i\n\n",t_id,eta,sportelli.occupati,sportelli.attesa_over,sportelli.attesa_under);
    pthread_cond_broadcast(&sportelli.SPORTELLO_LIBERATO);
    pthread_mutex_unlock(&sportelli.m);

    pthread_exit(NULL);
}