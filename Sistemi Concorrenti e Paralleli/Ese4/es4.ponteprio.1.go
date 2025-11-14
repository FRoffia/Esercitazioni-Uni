// ponte_sensounico_prio project main.go -- priorità di direzione ai veicoli da NORD
// ponte_senso_unico project main.go

package main

import (
	"fmt"
	"math/rand"
	"time"
)

const MAXBUFF = 100
const MAXPROC = 100
const MAX = 5 // capacità
const N int = 0
const S int = 1

var done = make(chan bool)
var termina = make(chan bool)
var entrataN = make(chan int, MAXBUFF) // necessità di accodamento per priorità
var entrataS = make(chan int, MAXBUFF) // necessità di accodamento per priorità
var uscitaN = make(chan int)
var uscitaS = make(chan int)
var ACK_N [MAXPROC]chan int //risposte client nord
var ACK_S [MAXPROC]chan int //risposte client sud
var r int

func veicolo(myid int, dir int) {
	var tt int
	tt = rand.Intn(5) + 1
	fmt.Printf("inizializzazione veicolo  %d direzione %d in secondi %d \n", myid, dir, tt)
	time.Sleep(time.Duration(tt) * time.Second)
	if dir == N {

		entrataN <- myid // send asincrona
		<-ACK_N[myid]    // attesa x sincronizzazione
		fmt.Printf("[veicolo %d]  entrato sul ponte in direzione  NORD\n", myid)
		tt = rand.Intn(5)
		time.Sleep(time.Duration(tt) * time.Second)
		uscitaN <- myid
		fmt.Printf("[veicolo %d]  uscito dal ponte in direzione  NORD\n", myid)
	} else {
		entrataS <- myid
		<-ACK_S[myid] // attesa x sincronizzazione
		fmt.Printf("[veicolo %d]  entrato sul ponte in direzione  SUD\n", myid)
		tt = rand.Intn(5)
		time.Sleep(time.Duration(tt) * time.Second)
		uscitaS <- myid
		fmt.Printf("[veicolo %d]  uscito dal ponte in direzione  SUD\n", myid)
	}
	done <- true
}

func server() {

	var contN int = 0
	var contS int = 0
	var sospN int = 0
	var sospS int = 0
	var sospesiN [MAXPROC]bool
	var sospesiS [MAXPROC]bool

	for {

		select {
		case x := <-entrataN:
			if (contN < MAX) && (contS == 0) {
				contN++
				ACK_N[x] <- 1 // risposta
			} else { //attende
				sospN++
				sospesiN[x] = true
			}
		case x := <-entrataS:
			if (contS < MAX) && (contN == 0) && (sospN == 0) {
				contS++
				ACK_S[x] <- 1 // termine "call"
			} else { //attende
				sospS++
				sospesiS[x] = true
			}
		case <-uscitaN:
			contN--
			if sospN > 0 {
				for i := 0; i < MAXPROC; i++ {
					if sospesiN[i] == true {
						sospN--
						sospesiN[i] = false
						contN++
						ACK_N[i] <- 1
						break
					}
				}
			} else if contN == 0 && sospS > 0 && sospN == 0 {
				for i := 0; i < MAXPROC && sospS > 0 && contS < MAX; i++ {
					if sospesiS[i] == true {
						sospS--
						sospesiS[i] = false
						contS++
						ACK_S[i] <- 1
					}
				}
			}
		case <-uscitaS:
			contS--
			if sospN > 0 && contS == 0 {
				for i := 0; i < MAXPROC && contN < MAX; i++ { // ciclo di risposte
					if sospesiN[i] == true {
						sospN--
						sospesiN[i] = false
						contN++
						ACK_N[i] <- 1
					}
				}
			} else if sospS > 0 && sospN == 0 {
				for i := 0; i < MAXPROC && sospS > 0; i++ {
					if sospesiS[i] == true {
						sospS--
						sospesiS[i] = false
						contS++
						ACK_S[i] <- 1
						break
					}
				}
			}
			// aggiungere risvegli
		case <-termina: // quando tutti i processi hanno finito
			fmt.Println("FINE !!!!!!")
			done <- true
			return
		}

	}
}

func main() {
	var VN int
	var VS int

	fmt.Printf("\n quanti veicoli NORD (max %d)? ", MAXPROC)
	fmt.Scanf("%d", &VN)
	fmt.Printf("\n quanti veicoli SUD (max %d)? ", MAXPROC)
	fmt.Scanf("%d", &VS)

	//inizializzazione canali
	for i := 0; i < VN; i++ {
		ACK_N[i] = make(chan int, MAXBUFF)
	}

	//inizializzazione canali
	for i := 0; i < VS; i++ {
		ACK_S[i] = make(chan int, MAXBUFF)
	}

	rand.Seed(time.Now().Unix())
	go server()

	for i := 0; i < VS; i++ {
		go veicolo(i, S)
	}
	for i := 0; i < VN; i++ {
		go veicolo(i, N)
	}

	for i := 0; i < VN+VS; i++ {
		<-done
	}
	termina <- true
	<-done
	fmt.Printf("\n HO FINITO ")
}
