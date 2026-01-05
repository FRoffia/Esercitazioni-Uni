#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>

#define N 2 //posti della biblioteca
#define STUDENTI 10 //studenti che vogliono accedere





typedef struct{
    int occupati_t;//studenti t in libreria
    int occupati_m;//studenti m in libreria
    int attesa_t_l;//laureandi t
    int attesa_t;//t non laureandi
    int attesa_m_l;//laureandi m
    int attesa_m;//m non laureandi
    pthread_mutex_t m;
    pthread_cond_t POSTO_LIBERO;
} Biblioteca;

typedef struct{
    int documenti[STUDENTI];
    int occupata;
    pthread_mutex_t m;
    pthread_cond_t PORTINERIA_LIBERA;
}Portineria;

Biblioteca biblioteca;
Portineria portineria;





void* studia(void* arg);





int main(){
    srand(time(NULL));

    //===========================< Inizializzazione risorse condivise >================================================================
    //biblioteca
    biblioteca.occupati_m = 0;
    biblioteca.occupati_t = 0;
    biblioteca.attesa_m = 0;
    biblioteca.attesa_t = 0;
    biblioteca.attesa_m_l = 0;
    biblioteca.attesa_t_l = 0;
    pthread_mutex_init(&biblioteca.m,NULL);
    pthread_cond_init(&biblioteca.POSTO_LIBERO,NULL);

    //portineria
    for(int i = 0; i < STUDENTI; i++){portineria.documenti[i] = 0;}
    portineria.occupata = 0;
    pthread_mutex_init(&portineria.m,NULL);
    pthread_cond_init(&portineria.PORTINERIA_LIBERA,NULL);

    

    //===========================< Creazione threads studenti >================================================================
    pthread_t studenti[STUDENTI];
    int rc = 0;

    for(int i = 0; i < STUDENTI; i++){
        rc = pthread_create(&studenti[i],NULL,studia,i);
        if(rc){
            printf("Errore durante la creazione del thread studente %i. Codice:%i\n",i,rc);
            exit(-1);
        }
    }



    //===========================< Join threads studenti >================================================================
    for(int i = 0; i < STUDENTI; i++){
        rc = pthread_join(studenti[i],NULL);
        if(rc){
            printf("Errore durante la terminazione del thread studente %i. Codice:%i\n",i,rc);
            exit(-1);
        }
    }



    //===========================< Controllo che tutti i documenti siano stati ripresi >================================================================
    for(int i = 0; i < STUDENTI; i++){
        if(portineria.documenti[i]){printf("PERFOOOOOOOOOOOOOOOOOOOOOOOOOORZAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA %i\n",i);}
    }
    
    printf("Tutti i threads sono terminati.\n");
}






void* studia(void* arg){
    int t_id = (int)arg;
    int l = rand()%2; //1 laureando, 0 no
    int mt = rand()%2; //1 magistrale, 0 triennale
    char* corso = mt ? "magistrale" : "triennale";
    char* situazione = l ? "laureando " : "";



    //===========================< Attesa per entrare in portineria >================================================================
    pthread_mutex_lock(&portineria.m);
    while(portineria.occupata){pthread_cond_wait(&portineria.PORTINERIA_LIBERA,&portineria.m);}
    portineria.occupata = 1;
    printf("Studente %i entrato in portineria per consegnare documento.\n\n",t_id);
    pthread_mutex_unlock(&portineria.m);



    //===========================< Consegna documento e svuotamento portineria >================================================================
    //sleep(1);
    pthread_mutex_lock(&portineria.m);
    if(portineria.documenti[t_id]) {
        printf("Errore, documento %i già presente in portineria\n",t_id);
        exit(-1);
    } else {
        portineria.documenti[t_id] = 1;
    }
    portineria.occupata = 0;
    printf("Studente %i ha consegnato il documento e libera la portineria.\n\n",t_id);
    pthread_cond_broadcast(&portineria.PORTINERIA_LIBERA);
    pthread_mutex_unlock(&portineria.m);



    //===========================< Attesa per entrare in biblioteca a studiare (non so se ci rendiamo conto neanche dovesse aspettare per ricevere dei soldi no gli tocca anche aspettare per andare a studiare io non lo so come se fosse una cosa che la gente fa volentieri boh....) >================================================================
    pthread_mutex_lock(&biblioteca.m);
    int studenti_attuali = biblioteca.occupati_m + biblioteca.occupati_t;
    
    if(mt){
        if(l){//caso magistrale laureando
            while(studenti_attuali == N || (biblioteca.occupati_m > biblioteca.occupati_t && biblioteca.attesa_t + biblioteca.attesa_t_l)){//controllo capacità massima e se ci sono già più studenti magistrali che triennali con studenti triennali in attesa
                biblioteca.attesa_m_l ++;
                pthread_cond_wait(&biblioteca.POSTO_LIBERO,&biblioteca.m);
                studenti_attuali = biblioteca.occupati_m + biblioteca.occupati_t;
                biblioteca.attesa_m_l --;
            }
        } else {//caso magistrale non laureando
            while(studenti_attuali == N || (biblioteca.occupati_m > biblioteca.occupati_t && biblioteca.attesa_t + biblioteca.attesa_t_l) || biblioteca.attesa_m_l){//controllo capacità massima, se ci sono già più studenti magistrali che triennali con studenti triennali in attesa e se ci sono studenti magistrali laureandi in attesa
                biblioteca.attesa_m ++;
                pthread_cond_wait(&biblioteca.POSTO_LIBERO,&biblioteca.m);
                studenti_attuali = biblioteca.occupati_m + biblioteca.occupati_t;
                biblioteca.attesa_m --;
            }
        }
        biblioteca.occupati_m ++;
    } else {
        if(l){//caso triennale laureando
            while(studenti_attuali == N || (biblioteca.occupati_t >= biblioteca.occupati_m && biblioteca.attesa_m + biblioteca.attesa_m_l)){//controllo capacità massima e se ci sono già più studenti triennali che magistrali con studenti magistrali in attesa
                biblioteca.attesa_t_l ++;
                pthread_cond_wait(&biblioteca.POSTO_LIBERO,&biblioteca.m);
                studenti_attuali = biblioteca.occupati_m + biblioteca.occupati_t;
                biblioteca.attesa_t_l --;
            }
        } else {//caso triennale non laureando
            while(studenti_attuali == N || (biblioteca.occupati_t >= biblioteca.occupati_m && biblioteca.attesa_m + biblioteca.attesa_m_l) || biblioteca.attesa_t_l){//controllo capacità massima, se ci sono già più studenti magistrali che triennali con studenti triennali in attesa e se ci sono studenti magistrali laureandi in attesa
                biblioteca.attesa_t ++;
                pthread_cond_wait(&biblioteca.POSTO_LIBERO,&biblioteca.m);
                studenti_attuali = biblioteca.occupati_m + biblioteca.occupati_t;
                biblioteca.attesa_t --;
            }
        }
        biblioteca.occupati_t ++;
    }
    printf("Studente %i %s%s entrato in biblioteca.\nAttesa m_l:%i\nAttesa m:%i\nAttesa t_l:%i\nAttesa t:%i\nOccupati m:%i\nOccupati t:%i\n\n",t_id,situazione,corso,biblioteca.attesa_m_l,biblioteca.attesa_m,biblioteca.attesa_t_l,biblioteca.attesa_t,biblioteca.occupati_m,biblioteca.occupati_t);
    pthread_mutex_unlock(&biblioteca.m);



    //===========================< Studio in biblioteca e uscita >================================================================
    sleep(10);//studio in biblioteca
    pthread_mutex_lock(&biblioteca.m);
    if(mt){
        biblioteca.occupati_m --;
    } else {
        biblioteca.occupati_t --;
    }
    printf("Studente %i, %s%s uscito dalla biblioteca.\nAttesa m_l:%i\nAttesa m:%i\nAttesa t_l:%i\nAttesa t%i:\nOccupati m:%i\nOccupati t:%i\n\n",t_id,situazione,corso,biblioteca.attesa_m_l,biblioteca.attesa_m,biblioteca.attesa_t_l,biblioteca.attesa_t,biblioteca.occupati_m,biblioteca.occupati_t);
    pthread_cond_broadcast(&biblioteca.POSTO_LIBERO);
    pthread_mutex_unlock(&biblioteca.m);



    //===========================< Attesa portineria 2 >================================================================
    pthread_mutex_lock(&portineria.m);
    while(portineria.occupata){pthread_cond_wait(&portineria.PORTINERIA_LIBERA,&portineria.m);}
    portineria.occupata = 1;
    printf("Studente %i entrato in portineria per riprendere documento.\n\n",t_id);
    pthread_mutex_unlock(&portineria.m);



    //===========================< Prendo documento e si va a letto >================================================================
    //sleep(1);
    pthread_mutex_lock(&portineria.m);
    if(!portineria.documenti[t_id]) {
        printf("Errore, documento non %i presente in portineria\n",t_id);
        exit(-1);
    } else {
        portineria.documenti[t_id] = 0;
    }
    portineria.occupata = 0;
    printf("Studente %i ha preso il documento e libera la portineria (a letto).\n\n",t_id);
    pthread_cond_broadcast(&portineria.PORTINERIA_LIBERA);
    pthread_mutex_unlock(&portineria.m);

    pthread_exit(NULL);
}