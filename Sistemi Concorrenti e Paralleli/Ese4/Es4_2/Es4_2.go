package main

import (
	"fmt"
	"math/rand"
	"time"
)

const TOT = 100
const MAXP = 20
const MAXC = 20

// tutto invio sincrono mi sa
var insPA = make(chan int)
var insPB = make(chan int)
var insCA = make(chan int)
var insCB = make(chan int)

var prelPA = make(chan int)
var prelCA = make(chan int)
var prelPB = make(chan int)
var prelCB = make(chan int)

var done = make(chan bool, 7)
var termina = make(chan bool, 5)

var tot int = 0

func when(b bool, c chan int) chan int {
	if !b {
		return nil
	}
	return c
}

func nastroPA() {
	var prod int = 0

	for {
		select {
		case <-termina:
			fmt.Printf("nastro PA fermato\n")
			done <- true
			return
		default:
			if prod < tot*4 {
				var tt int
				tt = rand.Intn(3) + 1
				time.Sleep(time.Duration(tt) * time.Second)
				insPA <- 1
				fmt.Printf("+ pneumatico A\n")
				prod++

			}
		}

	}
}

func nastroPB() {
	var prod int = 0

	for {
		select {
		case <-termina:
			fmt.Printf("nastro PB fermato\n")
			done <- true
			return
		default:
			if prod < tot*4 {
				var tt int
				tt = rand.Intn(3) + 1
				time.Sleep(time.Duration(tt) * time.Second)
				insPB <- 1
				fmt.Printf("+ pneumatico B\n")
				prod++

			}
		}

	}
}

func nastroCA() {
	var prod int = 0

	for {
		select {
		case <-termina:
			fmt.Printf("nastro CA fermato\n")
			done <- true
			return
		default:
			if prod < tot*4 {
				var tt int
				tt = rand.Intn(3) + 1
				time.Sleep(time.Duration(tt) * time.Second)
				insCA <- 1
				fmt.Printf("+ cerchione A\n")
				prod++

			}
		}

	}
}

func nastroCB() {
	var prod int = 0

	for {
		select {
		case <-termina:
			fmt.Printf("nastro CB fermato\n")
			done <- true
			return
		default:
			if prod < tot*4 {
				var tt int
				tt = rand.Intn(3) + 1
				time.Sleep(time.Duration(tt) * time.Second)
				insCB <- 1
				fmt.Printf("+ cerchione B\n")
				prod++
			}
		}

	}
}

func robotA() {
	for i := 0; i < tot; i++ {
		for j := 0; j < 4; j++ {
			//mando j così so quanti pezzi ho già montato???
			prelCA <- j
			<-prelCA
			var tt int
			tt = rand.Intn(5) + 1
			time.Sleep(time.Duration(tt) * time.Second)
			fmt.Printf("montato cerchione %d su auto A%d \n", j, i)
			prelPA <- j
			<-prelPA
			tt = rand.Intn(5) + 1
			time.Sleep(time.Duration(tt) * time.Second)
			fmt.Printf("montato pneumatico %d su auto A%d \n", j, i)
		}
		fmt.Printf("=== finita auto A%d \n", i)
	}
	fmt.Printf("=== Termina esecuzione robot A\n")
	done <- true
}

func robotB() {
	for i := 0; i < tot; i++ {
		for j := 0; j < 4; j++ {
			//mando j così so quanti pezzi ho già montato???
			prelCB <- j
			<-prelCB
			var tt int
			tt = rand.Intn(5) + 1
			time.Sleep(time.Duration(tt) * time.Second)
			fmt.Printf("montato cerchione %d su auto B%d \n", j, i)
			prelPB <- j
			<-prelPB
			tt = rand.Intn(5) + 1
			time.Sleep(time.Duration(tt) * time.Second)
			fmt.Printf("montato pneumatico %d su auto B%d \n", j, i)
		}
		fmt.Printf("=== finita auto B%d \n", i)
	}
	fmt.Printf("=== Termina esecuzione robot B\n")
	done <- true
}

func server() {
	var depPA int = 0
	var depPB int = 0
	var depCA int = 0
	var depCB int = 0

	var complA int = 0
	var complB int = 0

	for {
		select {

		//====================| inserimenti dai nastri |==========================================================
		case <-when((complA <= complB) && (depPA+depPB < MAXP), insPA):
			depPA++
		case <-when((complA <= complB) && (depCA+depCB < MAXC), insCA):
			depCA++
		case <-when((complB <= complA) && (depPA+depPB < MAXP), insPB):
			depPB++
		case <-when((complB <= complA) && (depCA+depCB < MAXC), insCB):
			depCB++

		//====================| prelievi dei robot |================================================================
		case <-when((complA <= complB) && (depPA > 0), prelPA):
			complA++
			prelPA <- 1
			depPA--
		case <-when((complA <= complB) && (depCA > 0), prelCA):
			prelCA <- 1
			depCA--
		case <-when((complB <= complA) && (depPB > 0), prelPB):
			complB++
			prelPB <- 1
			depPB--
		case <-when((complB <= complA) && (depCB > 0), prelCB):
			prelCB <- 1
			depCB--

		case <-termina: // quando tutti i processi hanno finito
			fmt.Println("FINE server !!!!!!")
			done <- true
			return
		}
	}
}

func main() {

	fmt.Printf("\n quante auto da produrre (max %d)? ", TOT)
	fmt.Scanf("%d", &tot)

	rand.Seed(time.Now().Unix())
	go server()
	go nastroPA()
	go nastroPB()
	go nastroCA()
	go nastroCB()
	go robotA()
	go robotB()

	//i due robot
	<-done
	<-done

	//per fermare il sercver e i nastri
	termina <- true
	termina <- true
	termina <- true
	termina <- true
	termina <- true

	//server e 4 nastri
	<-done
	<-done
	<-done
	<-done
	<-done

	fmt.Printf("\n HO FINITO \n")
}
