// pool di risorse equivalenti con guardie logiche

package main

import (
	"fmt"
	"math/rand"
	"time"
)

type Request struct {
	cli_id   int
	req_type string
}

type Resource struct {
	res_id   int
	res_type string
}

const MAXPROC = 2000

const NB = 100
const NE = 100

var richiesta = make(chan Request) //canali richiesta
var rilascio = make(chan Resource) //canali rilascio
var risorsa [MAXPROC]chan Resource //canali risorse?????
var done = make(chan int)          //canalus canalus
var termina = make(chan int)

func when(b bool, c chan int) chan int {
	if !b {
		return nil
	}
	return c
}

func client(i int) {

	var r_type int = rand.Intn(3)

	var r Request
	r.cli_id = i

	switch r_type {
	case 0:
		r.req_type = "BT"
	case 1:
		r.req_type = "EB"
	case 2:
		r.req_type = "FLEX"
	}

	richiesta <- r

	res := <-risorsa[i]

	fmt.Printf("\n [client %d] uso della risorsa %s, res_id: %d\n", i, res.res_type, res.res_id)
	rilascio <- res
	done <- i
}

func server(nb, ne int) {

	var disponibili_t int = nb
	var disponibili_e int = ne
	var i, j int
	var res Resource
	var libera_t [NB]bool
	var libera_e [NE]bool

	var attesa_t [MAXPROC]int
	var attesa_e [MAXPROC]int

	var in_attesa_t int = 0
	var in_attesa_e int = 0

	for i := 0; i < nb; i++ {
		libera_t[i] = true
	}

	for i := 0; i < ne; i++ {
		libera_e[i] = true
	}

	for {
		time.Sleep(time.Second * 1)
		fmt.Println("nuovo ciclo server")
		select {

		//=========================<< RILASCIO >>========================================
		case res = <-rilascio:
			switch res.res_type {
			case "bici elettrica":
				disponibili_e++
				libera_e[res.res_id] = true
				fmt.Printf("[server]  restituita risorsa %s, res_id: %d\n", res.res_type, res.res_id)

				if in_attesa_e > 0 {
					for i = MAXPROC - 1; i >= 0 && attesa_e[i] == 0; i++ {
					}

					for j = 0; j < nb && !libera_e[j]; j++ {
					}
					libera_e[j] = false
					disponibili_e--

					var res Resource
					res.res_id = j
					res.res_type = "bici elettrica"

					risorsa[attesa_e[i]] <- res
					fmt.Printf("[server]  allocata risorsa %s, res_id: %d a cliente %d \n", res.res_type, j, attesa_e[i])

					attesa_e[i] = 0
					in_attesa_e--
				}

			case "bici":
				disponibili_t++
				libera_t[res.res_id] = true
				fmt.Printf("[server]  restituita risorsa %s, res_id: %d\n", res.res_type, res.res_id)

				if in_attesa_t > 0 {
					for i = MAXPROC - 1; i >= 0 && attesa_t[i] == 0; i++ {
					}

					for j = 0; j < nb && !libera_t[j]; j++ {
					}
					libera_t[j] = false
					disponibili_t--

					var res Resource
					res.res_id = j
					res.res_type = "bici"

					risorsa[attesa_t[i]] <- res
					fmt.Printf("[server]  allocata risorsa %s, res_id: %d a cliente %d \n", res.res_type, j, attesa_t[i])

					attesa_t[i] = 0
					in_attesa_t--
				}
			}

		//=========================<< RICHIESTA >>========================================
		case req := <-richiesta:
			switch req.req_type {
			case "BT":
				if disponibili_t > 0 {
					for i = 0; i < nb && !libera_t[i]; i++ {
					}
					libera_t[i] = false
					disponibili_t--

					var res Resource
					res.res_id = i
					res.res_type = "bici"

					risorsa[req.cli_id] <- res
					fmt.Printf("[server]  allocata risorsa %s, res_id: %d a cliente %d \n", res.res_type, i, req.cli_id)
				} else {
					for i = 0; i < MAXPROC && attesa_t[i] == 0; i++ {
					}
					attesa_t[i] = req.cli_id
					in_attesa_t++
				}
			case "EB":
				if disponibili_e > 0 {
					for i = 0; i < nb && !libera_e[i]; i++ {
					}
					libera_e[i] = false
					disponibili_e--

					var res Resource
					res.res_id = i
					res.res_type = "bici elettrica"

					risorsa[req.cli_id] <- res
					fmt.Printf("[server]  allocata risorsa %s, res_id: %d a cliente %d \n", res.res_type, i, req.cli_id)
				} else {
					for i = 0; i < MAXPROC && attesa_e[i] == 0; i++ {
					}
					attesa_e[i] = req.cli_id
					in_attesa_e++
				}
			case "FLEX":
				if disponibili_e > 0 {
					for i = 0; i < nb && !libera_e[i]; i++ {
					}
					libera_e[i] = false
					disponibili_e--

					var res Resource
					res.res_id = i
					res.res_type = "bici elettrica"

					risorsa[req.cli_id] <- res
					fmt.Printf("[server]  allocata risorsa %s, res_id: %d a cliente %d \n", res.res_type, i, req.cli_id)
				} else if disponibili_t > 0 {
					for i = 0; i < nb && !libera_t[i]; i++ {
					}
					libera_t[i] = false
					disponibili_t--

					var res Resource
					res.res_id = i
					res.res_type = "bici"

					risorsa[req.cli_id] <- res
					fmt.Printf("[server]  allocata risorsa %s, res_id: %d a cliente %d \n", res.res_type, i, req.cli_id)
				} else {
					for i = 0; i < MAXPROC && attesa_e[i] == 0; i++ {
					}
					attesa_e[i] = req.cli_id
					in_attesa_e++
				}
			}

		//=========================<< TERMINA >>========================================
		case <-termina: // quando tutti i processi clienti hanno finito
			fmt.Println("Il server ha finito  !")
			done <- 1
			return

		}
	}
}

func main() {
	var cli, nb, ne int
	fmt.Printf("\n quanti clienti (max %d)? ", MAXPROC)
	fmt.Scanf("%d", &cli)
	fmt.Println("clienti:", cli)
	fmt.Printf("\n quante bici tradizionali (max %d)? ", NB)
	fmt.Scanf("%d", &nb)
	fmt.Printf("\n quante bici elettriche (max %d)? ", NE)
	fmt.Scanf("%d", &ne)
	fmt.Println("risorse da gestire:%d", nb+ne)

	//inizializzazione canali
	for i := 0; i < MAXPROC; i++ {
		risorsa[i] = make(chan Resource)
	}

	for i := 0; i < cli; i++ {
		go client(i)
	}
	go server(nb, ne)

	for i := 0; i < cli; i++ {
		<-done
	}
	termina <- 1 //terminazione server
	<-done
}
