/**
 * @file server.c
 * @brief Main del programma.
 *
 * Si dichiara che il contenuto di questo file è in ogni sua parte
 * opera originale dell'autore.
 *
 * @author 	Lorenzo Beretta, 536242, loribere@gmail.com
 */
#include <string.h>
#include <sys/mman.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>

#include "connections.h"
#include "queue.h"
#include "ops.h"	
#include "hashtable.h"
#include "stat.h"
#include "parser.h"

// header della funzione worker, implementata in "worker.c"
void* worker(void*);

// statistiche sul server
stat_set_t sset;

// coda condivisa tra tutti i threads per le richieste dei clients
queue_t queue;

// hash table condivisa contenente i nicknames registrati
hash_t* htab;

/*--------costanti lette nel file di configurazione--------------*/ 
char* UnixPath;
char* DirName;
char* StatFileName;
int MaxConnections;
int ThreadsInPool;
int MaxHistMsgs;
int MaxFileSize;
int MaxMsgSize;
/*---------------------------------------------------------------*/

/*----------- Gestione dei fd per l'ascolto dei client ----------*/ 

// connected_fd[i] != 0 sse il fd #i è nuovamente disponibile all'ascolto  
int* connected_fd;

// associa a fd il nickname connesso su tale file descriptor se esiste, NUll altrimenti
char** fd_to_nick; 

// mutex per sincronizzare le strutture connected_fd e fd_to_nick
pthread_mutex_t connected_mutex;

/*-----------------------------------------------------------------*/

// flag per far terminare tutti i threads
char loop_interrupt = 0;

/*------------------ MAIN DEL THREAD listener_th-----------*/

void* listener(void* useless_arg) {
	char* useless_buf[ThreadsInPool];
	int max_fd = 5;
	
	// inizializzo il set dei fd da asoltare
	fd_set set, rdset;
	FD_ZERO(&set);
	FD_SET(3, &set);
	FD_SET(4, &set);
	
	listen(3, SOMAXCONN);
	
	// ciclo di ascolto
	while (!loop_interrupt) {
		// assegno a rdset i fd disponibili per I/O
		rdset = set;
		if (select(max_fd + 1, &rdset, NULL, NULL, NULL) < 0) {
			perror("Errore nella select del listener\n");
			exit(EXIT_FAILURE);
		}
		
		// Comunicazione interna sulla pipe
		if (FD_ISSET(4, &rdset)) {
			// Leggo dati inutili, il loro scopo è solo 
			// rendere il fd read della pipe selezionabile
			// per segnalare che ci sono nuovi fd liberi
			if (read(4, useless_buf, ThreadsInPool) < 0) {
				perror("Errore leggendo la Pipe\n");
				exit(EXIT_FAILURE);
			}
			
			// Rimetto nel mio set i fd da ascoltare
			pthread_mutex_lock(&connected_mutex);
			for (int i = 6; i < MaxConnections + 6; i++) {
				if (!connected_fd[i]) continue;
				FD_SET(i, &set);
				connected_fd[i] = 0;
			}
			pthread_mutex_unlock(&connected_mutex);
		}
		
		// Nuova connessione al socket
		if (FD_ISSET(3, &rdset)) {
			int new_fd = accept(3, NULL, NULL);
			
			// caso in cui c'è un tentativo di connessione
			// oltre il limite, gli chiudo il file e lo conto
			// tra gli errori nelle statistiche dato che in ops.h
			// non c'è un messaggio di errore previsto per questo
			if (new_fd > 5 + MaxConnections) {
				update_stat(&sset, messaggi_di_errore);
				close(new_fd);
			} else {
				FD_SET(new_fd, &set);
				max_fd = (max_fd > new_fd) ? (max_fd) : (new_fd);
			}
		}
		
		// Operazione su una connessione già stabilita
		for (int i = 6; i <= max_fd; i++) {
			if (!FD_ISSET(i, &rdset)) continue;
			FD_CLR(i, &set);
			push_queue(&queue, i);
		}
	}		
	return NULL;
}
/*--------------------------------------------------------------------------*/

/*---------------------------------MAIN DEL SERVER--------------------------*/

int main(int argc, char* argv[]) {
	
/*-------------------------PARSING INIZIALE---------------------------------*/
	
	// controllo sull'inserimento degli argomenti
	if (argc < 3 || strlen(argv[1]) != 2 || strncmp(argv[1], "-f", 2)) {
		fprintf(stderr, 
			"Lanciare il server utlizzando il comando:\n"
			"%s -f <file_di_configurazione>\n", argv[0]
		);
	}
	
	// effettuo il parsing del file di configurazione
	parse(argv[2]);
	
/*----------------------------------------------------------------------------*/

/*-------------------INIZIALIZZAZIONE STURTTURE CONDIVISE---------------------*/
	
	
	// Alloco ed inizializzo connected_fd a zero
	if ((connected_fd = calloc(MaxConnections + 6, sizeof(int))) < 0) {
		perror("Errore calloc\n");
		return -1;
	}
	
	// inizializzo fd_to_nick
	if ((fd_to_nick = calloc(MaxConnections + 6, sizeof(char*))) < 0) {
		perror("Errore calloc\n");
		return -1;
	}
	
	// inizializzo i mutex
	pthread_mutex_init(&(sset.mutex), NULL);
	pthread_mutex_init(&connected_mutex, NULL);
	
	// creo le strutture dati comuni a tutti i thread
	queue = create_queue(MAX_QUEUE_LEN);
	htab = create_hash(MaxHistMsgs, HASH_NBUCKETS);
	
	// inizializzo i file descriptors per il socket
	int sock_fd;
	if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		perror("Errore creando il Socket\n");
		return -1;
	}
	
	// reidnirizzo il file descriptor del socket su 3
	if (sock_fd != 3) {
		if (dup2(sock_fd, 3) < 0) {
			perror("Errore reindirizzando sock_fd su 3\n");
			return -1;
		}
		close(sock_fd);
	}
	
	// lego il socket all'indirizzo UnixPath
	struct sockaddr_un sa;
	sa.sun_family = AF_UNIX;
	strncpy(sa.sun_path, UnixPath, strlen(UnixPath) + 1);
	if (bind(3,  (struct sockaddr*) &sa, sizeof(sa)) < 0) {
		perror("Errore binding Socket\n");
		return -1;
	}
		
	// inizializzo il fd per la pipe
	int pipe_fd[2];
	if (pipe(pipe_fd) < 0) {
		perror("Errore creazione pipe\n");
		return -1;
	}
	
	// reindirizzo pipe read e write sui fd 4 e 5 rispettivamente
	if (pipe_fd[0] != 4) {
		if (dup2(pipe_fd[0], 4) < 0) {
			perror("Errore reindirizzamento pipe_fd[0] read su 4\n");
			return -1;
		}
		close(pipe_fd[0]);
	}
	if (pipe_fd[1] != 5) {
		if (dup2(pipe_fd[1], 5) < 0) {
			perror("Errore reindirizzamento pipe_fd[1] write su 5\n");
			return -1;
		}
		close(pipe_fd[1]);
	}
/*----------------------------------------------------------------------------*/

/*---------------------------GESTIONE DEI SEGNALI--------------------------------*/
	
	// ignoro SIGPIPE per non avere problemi in caso un client riattacchi improvvisamente
	struct sigaction no_sig_pipe;
	memset(&no_sig_pipe, 0, sizeof(no_sig_pipe));
	no_sig_pipe.sa_handler = SIG_IGN;
	if (sigaction(SIGPIPE, &no_sig_pipe, NULL) < 0) {
		perror("Errore di sigaction\n");
		return -1;
	}
	
	// maschero tutti i segnali da gestire attraverso la sigwait
	sigset_t signalmask;
	if (sigemptyset(&signalmask) < 0) {
		perror("sigemptyset\n");
		return -1;
	}
	if (sigaddset(&signalmask, SIGUSR1) < 0) {
		perror("sigaddset\n");
		return -1;
	}
	if (sigaddset(&signalmask, SIGINT) < 0) {
		perror("sigaddset\n");
		return -1;
	}
	if (sigemptyset(sigaddset(&signalmask, SIGTERM) < 0) < 0) {
		perror("sigaddset\n");
		return -1;
	}
	if (sigaddset(&signalmask, SIGQUIT) < 0) {
		perror("sigaddset\n");
		return -1;
	}
	if (pthread_sigmask(SIG_SETMASK, &signalmask, NULL) == -1) {
		perror("pthread_sigmask\n");
		return -1;
	}
	
	// loop di gestione dei segnali
	while (!loop_interrupt) {
		int sign;
		if (sigwait((sigset*) &signalmask, &sign) != 0) {
			perror("sigwait\n");
			exit(EXIT_FAILURE);
		}
		
		// segnale definito dall'utente: stampo le statistiche sul file
		if (sign == SIGUSR1) print_stat(sset, StatFileName);
		
		// segnale di terminazione: faccio terminare ogni thread 
		if (sign == SIGINT || sign == SIGTERM || sign == SIGQUIT) {
			
			// gli worker termineranno dopo aver eseguito tutti task dei clients in coda
			for (int i = 0; i < ThreadsInPool; i++) {
				push_queue(&queue, -1);
			}
			
			// interrompe i loop principali di ogn thread
			loop_interrupt = true;
			char* tmp_buf = "1";
			
			// scrive sulla pipe interna per fare uscire il listener dalla select
			if (write(4, tmp_buf, 1) < 0) {
				perror("Errore nelllos scrivere la pipe interna, riprovo\m");
				return -1;
			}
		}
	}	
/*----------------------------------------------------------------------------*/

/*-------------------CREAZIONE THREADS, SERVER A REGIME-----------------------*/
	
	// creo il thread listener
	pthread_t listener_th;
	pthread_create(&listener_th, NULL, &listener, NULL);
	
	// creo i thread worker
	pthread_t worker_th[ThreadsInPool];
	for  (int i = 0; i < ThreadsInPool; ++i) {
		pthread_create(&worker_th[i], NULL, &worker, NULL);
	}
/*----------------------------------------------------------------------------*/
	
/*----------------------------PULIZIA FINALE----------------------------------*/

	// Elimino il bind del Socket
	unlink(UnixPath);
	// Elimino i fd di socket e pipe
	close(3);
	close(4);
	close(5);	
	// Distruggo i mutex
	pthread_mutex_destroy(&(sset.mutex));
	// Dealloco queue e hash table
	destroy_hash(htab);
	destroy_queue(&queue);
	// Dealloco vettori allocati dinamicamente
	free(connected_fd);
	free(fd_to_nick);
		
	return 0;
}
