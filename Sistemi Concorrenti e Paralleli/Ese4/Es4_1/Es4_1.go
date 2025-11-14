// ponte_sensounico_prio project main.go -- priorità di direzione ai veicoli da NORD
// ponte_senso_unico project main.go

package main

import (
	"fmt"
	"math/rand"
	"time"
)

const MAXBUFF = 200
const MAXPROC = 200
const MAX = 20 // capacità
const N int = 0
const S int = 1

var done = make(chan bool)
var termina = make(chan bool)
var entrataN_V = make(chan int, MAXBUFF) // necessità di accodamento per priorità
var entrataS_V = make(chan int, MAXBUFF) // necessità di accodamento per priorità
var entrataN_P = make(chan int, MAXBUFF) // necessità di accodamento per priorità
var entrataS_P = make(chan int, MAXBUFF) // necessità di accodamento per priorità
var uscitaN = make(chan int)
var uscitaS = make(chan int)
var ACK_N [MAXPROC]chan int //risposte client nord
var ACK_S [MAXPROC]chan int //risposte client sud
var r int

func when(b bool, c chan int) chan int {
	if !b {
		return nil
	}
	return c
}

func veicolo(myid int, dir int) {
	var tt int
	tt = rand.Intn(5) + 1
	fmt.Printf("inizializzazione veicolo  %d direzione %d in secondi %d \n", myid, dir, tt)
	time.Sleep(time.Duration(tt) * time.Second)
	if dir == N {

		entrataN_V <- myid // send asincrona
		<-ACK_N[myid]      // attesa x sincronizzazione
		fmt.Printf("[veicolo %d]  entrato sul ponte in direzione  NORD\n", myid)
		tt = rand.Intn(5)
		time.Sleep(time.Duration(tt) * time.Second)
		uscitaN <- myid
		fmt.Printf("[veicolo %d]  uscito dal ponte in direzione  NORD\n", myid)
	} else {
		entrataS_V <- myid
		<-ACK_S[myid] // attesa x sincronizzazione
		fmt.Printf("[veicolo %d]  entrato sul ponte in direzione  SUD\n", myid)
		tt = rand.Intn(5)
		time.Sleep(time.Duration(tt) * time.Second)
		uscitaS <- myid
		fmt.Printf("[veicolo %d]  uscito dal ponte in direzione  SUD\n", myid)
	}
	done <- true
}

func pedone(myid int, dir int) {
	var tt int
	tt = rand.Intn(5) + 1
	fmt.Printf("inizializzazione pedone  %d direzione %d in secondi %d \n", myid, dir, tt)
	time.Sleep(time.Duration(tt) * time.Second)
	if dir == N {

		entrataN_P <- myid // send asincrona
		<-ACK_N[myid]      // attesa x sincronizzazione
		fmt.Printf("[pedone %d]  entrato sul ponte in direzione  NORD\n", myid)
		tt = rand.Intn(5)
		time.Sleep(time.Duration(tt) * time.Second)
		uscitaN <- myid
		fmt.Printf("[pedone %d]  uscito dal ponte in direzione  NORD\n", myid)
	} else {
		entrataS_P <- myid
		<-ACK_S[myid] // attesa x sincronizzazione
		fmt.Printf("[pedone %d]  entrato sul ponte in direzione  SUD\n", myid)
		tt = rand.Intn(5)
		time.Sleep(time.Duration(tt) * time.Second)
		uscitaS <- myid
		fmt.Printf("[pedone %d]  uscito dal ponte in direzione  SUD\n", myid)
	}
	done <- true
}

func server(vn int, vs int) {

	var contN int = 0
	var contS int = 0

	var veicoloN int = 0
	var veicoloS int = 0

	for {

		select {
		case x := <-when((contS < MAX) && (veicoloN == 0), entrataS_P): //entrata pedone sud
			contS++
			ACK_S[x] <- 1 // termine "call"

		case x := <-when((contN < MAX) && (veicoloS == 0) && (len(entrataS_P) == 0), entrataN_P): //entrata pedone nord
			contN++
			ACK_N[x] <- 1 // termine "call"

		case x := <-when((contS < MAX-9) && (contN == 0) && (len(entrataN_P) == 0) && (len(entrataS_P) == 0), entrataS_V): //entrata auto sud
			contS += 10
			ACK_S[x] <- 1 // termine "call"
			veicoloS++

		case x := <-when((contN < MAX-9) && (contS == 0) && (len(entrataS_V) == 0) && (len(entrataN_P) == 0) && (len(entrataS_P) == 0), entrataN_V): //entrata auto nord
			contN += 10
			ACK_N[x] <- 1 // termine "call"
			veicoloN++

		case x := <-uscitaN:
			if x >= vn {
				contN--
			} else {
				contN -= 10
				veicoloN--
			}

		case x := <-uscitaS:
			if x >= vs {
				contS--
			} else {
				contS -= 10
				veicoloS--
			}
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
	var PN int
	var PS int

	fmt.Printf("\n quanti veicoli NORD (max %d)? ", MAXPROC/4)
	fmt.Scanf("%d", &VN)
	fmt.Printf("\n quanti veicoli SUD (max %d)? ", MAXPROC/4)
	fmt.Scanf("%d", &VS)
	fmt.Printf("\n quanti pedoni NORD (max %d)? ", MAXPROC/4)
	fmt.Scanf("%d", &PN)
	fmt.Printf("\n quanti pedoni SUD (max %d)? ", MAXPROC/4)
	fmt.Scanf("%d", &PS)

	//inizializzazione canali
	for i := 0; i < VN+PN; i++ {
		ACK_N[i] = make(chan int, MAXBUFF/4)
	}

	//inizializzazione canali
	for i := 0; i < VS+PS; i++ {
		ACK_S[i] = make(chan int, MAXBUFF/4)
	}

	rand.Seed(time.Now().Unix())
	go server(VN, VS)

	for i := 0; i < VS; i++ {
		go veicolo(i, S)
	}
	for i := 0; i < VN; i++ {
		go veicolo(i, N)
	}
	for i := VS; i < VS+PS; i++ {
		go pedone(i, S)
	}
	for i := VN; i < VN+PN; i++ {
		go pedone(i, N)
	}

	for i := 0; i < VN+VS+PN+PS; i++ {
		<-done
	}
	termina <- true
	<-done
	fmt.Printf("\n HO FINITO ")
}
