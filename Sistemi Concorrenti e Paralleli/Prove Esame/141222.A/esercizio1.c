#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

#define MAX 2000
#define SFOGLINE 20
#define CLIENTI 100

 typedef struct{
    int attesa_prenotazioni_cliente;
    int attesa_prenotazioni_quantita;
    int attesa_ritiro_cliente;
    int attesa_consegna_sfoglina;
    int attesa_consegna_quantità;
    int tortellini_prenotati;
    int tortellini_pronti;
    int prenotazioni[CLIENTI];//quantità prenotate per ogni cliente, -1 se prenotazione già consegnata o invalida
    pthread_mutex_t m;
    pthread_cond_t PROSSIMO;
} Laboratorio;

Laboratorio lab;

void* gestisci_lab();
void* produci_tortellini();
void* compra_tortellini(void* arg);

int main(){
    //==============================< Inizializzazione Struttura >==========================================
    lab.attesa_consegna_quantità = 0;
    lab.attesa_prenotazioni_cliente = 0;
    lab.attesa_prenotazioni_quantita = 0;
    lab.attesa_ritiro_cliente = 0;
    lab.tortellini_prenotati = 0;
    lab.tortellini_pronti = 0;
    for(int i = 0; i < CLIENTI; i++){lab.prenotazioni[i] = 0;};
    pthread_mutex_init(&lab.m, NULL);
    pthread_cond_init(&lab.PROSSIMO, NULL);



    //==============================< Creazione Threads >==========================================
    pthread_t clienti[CLIENTI];
    pthread_t sfogline[SFOGLINE];
    pthread_t server;
    int rc = 0;

    rc = pthread_create(&server,NULL,gestisci_lab,NULL);
    if(rc){
        printf("Errore durante la creazione del thread server, errore: %i",rc);
        exit(-1);
    }

    for(int i = 0; i < CLIENTI; i++){
        rc = pthread_create(&clienti[i],NULL,compra_tortellini,i);
        if(rc){
            printf("Errore durante la creazione del thread cliente %i, errore: %i",i,rc);
            exit(-1);
        }
    }

    for(int i = 0; i < SFOGLINE; i++){
        rc = pthread_create(&sfogline[i],NULL,produci_tortellini,NULL);
        if(rc){
            printf("Errore durante la creazione del thread sfoglina %i, errore: %i",i,rc);
            exit(-1);
        }
    }



    //==============================< Attesa Threads e Chiusura >==========================================
    rc = pthread_join(server,NULL);
    if(rc){
        printf("Errore durante la join del thread server, errore: %i",rc);
        exit(-1);
    }

    for(int i = 0; i < CLIENTI; i++){
        rc = pthread_join(clienti[i],NULL);
        if(rc){
            printf("Errore durante la join del thread cliente %i, errore: %i",i,rc);
            exit(-1);
        }
    }

    for(int i = 0; i < SFOGLINE; i++){
        rc = pthread_join(sfogline[i],NULL);
        if(rc){
            printf("Errore durante la join del thread sfoglina %i, errore: %i",i,rc);
            exit(-1);
        }
    }

    printf("Tutti i threads terminati correttamente.\n");

}


void* gestisci_lab(){
    int done = 0;
    while(!done){
        pthread_mutex_lock(&lab.m);

        //==============================< Gestione richieste con priorità >==========================================
        if(lab.attesa_prenotazioni_quantita){//gestisco prenotazione in ogni caso

            if(lab.tortellini_prenotati + lab.attesa_prenotazioni_quantita <= MAX){//accetto ordine
                lab.tortellini_prenotati += lab.attesa_prenotazioni_quantita;
                lab.prenotazioni[lab.attesa_prenotazioni_cliente] = lab.attesa_prenotazioni_quantita;
                printf("Lab: prenotazione accettata, cliente %i per %i tortellini. Prenotazioni totali: %i su %i",lab.attesa_prenotazioni_cliente, lab.attesa_prenotazioni_quantita, lab.tortellini_prenotati, MAX);

            } else {//rifiuto ordine
                lab.prenotazioni[lab.attesa_prenotazioni_cliente] = -1;
                printf("Lab: prenotazione rifiutata, cliente %i per %i tortellini. Prenotazioni totali: %i su %i",lab.attesa_prenotazioni_cliente, lab.attesa_prenotazioni_quantita, lab.tortellini_prenotati, MAX);

            }

            lab.attesa_prenotazioni_cliente = 0;
            lab.attesa_prenotazioni_quantita = 0;

        } else if(lab.prenotazioni[lab.attesa_ritiro_cliente] > 0 && lab.prenotazioni[lab.attesa_ritiro_cliente] <= lab.tortellini_pronti){//se prenotazione è valida e disponibilità ok gestisco la richiesta
            lab.tortellini_pronti -= lab.prenotazioni[lab.attesa_ritiro_cliente];
            lab.prenotazioni[lab.attesa_ritiro_cliente] = -1;
            lab.attesa_ritiro_cliente = 0;

        } else if(lab.prenotazioni[lab.attesa_ritiro_cliente] == -1){//se la prenotazione non è valida cancello il numero e vado avanti
            lab.attesa_ritiro_cliente = 0;
    
        } else if(lab.attesa_consegna_quantità) {//caso consegna
            lab.tortellini_pronti += lab.attesa_consegna_quantità;
            lab.to
        }





        //==============================< Controllo prenotazioni e done >==========================================
        done = 1;
        for(int i = 0; i < CLIENTI; i++){if(lab.prenotazioni[i] >= 0){done = 0;}}



    }
}

void* produci_tortellini();
void* compra_tortellini(void* arg);