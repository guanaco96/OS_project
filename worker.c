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

// variabili esterne documentate in server.c
extern stat_set_t sset;
extern queue_t queue;
extern hash_t* htab;
extern char loop_interrupt;

extern int* connected_fd;
extern char** fd_to_nick;
extern pthread_mutex_t connected_mutex;

extern int MaxConnections;
extern int MaxMsgSize;
extern char* DirName;
extern int MaxFileSize;


// aggiuinge un nickname alla mappa fd_to_nickname
void add_nick(int fd, char* nick) {
	pthread_mutex_lock(&connected_mutex);
	if (fd_to_nick[fd] == NULL) {
		fd_to_nick[fd] = malloc((strlen(nick) + 1) * sizeof(char));
		strcpy(fd_to_nick[fd], nick);
	}
	pthread_mutex_unlock(&connected_mutex);
}

// invia alla connesione data da fd la lista dei client connessi
void send_list(int fd) {
	message_data_t data;
	data.buf = calloc(MaxConnections * (MAX_NAME_LENGTH + 1), sizeof(char));
	data.hdr.len = 0;
	char* ptr = data.buf;
	
	pthread_mutex_lock(&connected_mutex);
	for (int j = 6; j < MaxConnections + 6; j++) {
		if (fd_to_nick[j]) {
			strncpy(ptr, fd_to_nick[j], MAX_NAME_LENGTH + 1);
			ptr += MAX_NAME_LENGTH + 1;
			data.hdr.len += MAX_NAME_LENGTH + 1;
		}
	}	
	pthread_mutex_unlock(&connected_mutex);
	
	sendData(fd, &data);
	free(data.buf);
}

// invia una risposta al client contenente solo l'operazione
void answer(int fd, op_t op) {
	message_hdr_t reply;
	if (op == OP_FAIL ||
		op == OP_NICK_ALREADY ||
		op == OP_NICK_UNKNOWN ||
		op == OP_MSG_TOOLONG ||
		op == OP_NO_SUCH_FILE) {
			update_stat(&sset, messaggi_di_errore, 1);
		}
	
	reply.op = op;
	sendHeader(fd, &reply);
}

// inserisce il messaggio nella cronologia di tutti gli utenti registrati
void append_all(message_t msg) {
	int nbuckets = htab->ht->nbuckets;
	icl_entry_t** buckets = htab->ht->buckets;
	
	for (int i = 0; i < nbuckets; i++) {
		icl_entry_t* p = buckets[i];
		while (p) {
			nickname_t* nk = p->data;
			append_msg_nickname(nk, msg);
			p = p->next;
		}
	}
}
	
/*------------------------------------------------------------------------*/

/*--------------------------MAIN DEI THREAD worker_th---------------------*/

void* worker(void* useless_arg) {
	message_t msg;
	memset(&msg, 0, sizeof(message_t));
	
	while (!loop_interrupt) {
		// estrazione MT-safe dalla queue
		int fd = pop_queue(&queue);
		
		// far terminare il programma in caso di segnale
		if (fd < 0) return NULL;
		
		// leggo il messaggio e controllo se l'utente è ancora online
		if (readMsg(fd, &msg) == 0) {
			nickname_t* user = find_hash(htab, fd_to_nick[fd]);
			if (user) user->fd = -1;
			
			pthread_mutex_lock(&connected_mutex);
			if (fd_to_nick[fd]) {
				free(fd_to_nick[fd]);
				fd_to_nick[fd] = NULL;
			}
			pthread_mutex_unlock(&connected_mutex);
			
			close(fd);
			continue;
		}
				
		op_t op = msg.hdr.op;
		char* sender = msg.hdr.sender;
		char* receiver = msg.data.hdr.receiver;
		char* text = msg.data.buf;
		int text_len = msg.data.hdr.len;
		
		#ifdef DEBUG
			print_message(&msg);
		#endif
		
		// se il messaggio è troppo lungo
		if (text_len > MaxMsgSize) {
			answer(fd, OP_MSG_TOOLONG);
			break;
		}
		
		// eseguo l'operazione
		switch (op) {
	/*------------------------REGISTER_OP---------------------------------*/
		case REGISTER_OP: {
			// caso in cui il nick è già registrato
			if (find_hash(htab, sender) != NULL) {
				answer(fd, OP_NICK_ALREADY);
				break;
			}
			nickname_t* user = insert_hash(htab, sender);
			// se l'inserimento nella table fallisce
			if (user == NULL) {
				fprintf(stderr, "Errore inserimento hash table\n");
				answer(fd, OP_FAIL);
				break;
			}
			// se va tutto a buon fine
			user->fd = fd;
			add_nick(fd, sender);
			answer(fd, OP_OK);
			send_list(fd);
			update_stat(&sset, utenti_registrati, 1);
		} break;
	/*---------------------------CONNECT_OP------------------------------*/
		case CONNECT_OP: {
			nickname_t* user = find_hash(htab, sender);
			// se l'utente non risulta registrato
			if (user == NULL) {
				answer(fd, OP_NICK_UNKNOWN);
				break;
			}
			// se va tutto a buon fine
			user->fd = fd;
			add_nick(fd, sender);
			answer(fd, OP_OK);
			send_list(fd);			
		} break;
	/*-----------------------------POSTTXT_OP-----------------------------*/
		case POSTTXT_OP: {
			nickname_t* user1 = find_hash(htab, sender);
			nickname_t* user2 = find_hash(htab, receiver);
			// se uno dei nicknames non è riconosciuto
			if (!user1 || !user2) {
				answer(fd, OP_NICK_UNKNOWN);
				break;
			}
			
			msg.hdr.op = TXT_MESSAGE;
			
			// caso in cui il messaggio è troppo lungo
			if (msg.data.hdr.len >= MaxMsgSize) {
				answer(fd, OP_MSG_TOOLONG);
				break;
			}
			
			// se il ricevente è online il messaggio gli viene inviato
			// altrimenti viene soltanto salvato nella cronologia
			if (user2->fd != -1) {
				sendRequest(user2->fd, &msg);
				update_stat(&sset, messaggi_consegnati, 1);
			} else {
				update_stat(&sset, messaggi_da_consegnare, 1);
			}
			
			append_msg_nickname(user2, msg);
			answer(fd, OP_OK);
		} break;
	/*-----------------------------POSTTXTALL_OP--------------------------*/
		case POSTTXTALL_OP: {
			nickname_t* user = find_hash(htab, sender);
			
			// se il nickname non è riconosciuto
			if (user == NULL) {
				answer(fd, OP_NICK_UNKNOWN);
				break;
			}
			
			update_stat(&sset, messaggi_da_consegnare, sset.set[utenti_registrati]);
			msg.hdr.op = TXT_MESSAGE;
			// il messaggio viene recapitato ai client connessi
			pthread_mutex_lock(&connected_mutex);
			for (int i = 6; i < MaxConnections + 6; i++) {
				if (fd_to_nick[i] == NULL) continue;
				sendRequest(i, &msg);
				update_stat(&sset, messaggi_consegnati, 1);
				update_stat(&sset, messaggi_da_consegnare, -1);
			}
			pthread_mutex_unlock(&connected_mutex);
			// viene poi aggiunto alla history di tutti
			append_all(msg);
			answer(fd, OP_OK);
			
		} break;
	/*-------------------------POSTFILE_OP-------------------------------*/
		case POSTFILE_OP: {
			nickname_t* user1 = find_hash(htab, sender);
			nickname_t* user2 = find_hash(htab, receiver);
			// se uno dei nicknames non è riconosciuto
			if (!user1 || !user2) {
				answer(fd, OP_NICK_UNKNOWN);
				break;
			}
			// messaggio contenente la mmap del file
			message_t fmsg;
			if (readData(fd, &fmsg.data) < 0) {
				perror("readData");
				answer(fd, OP_FAIL);
				break;
			}
			char* mappedfile = fmsg.data.buf;
			int mappedsize = fmsg.data.hdr.len;
			
			// caso in cui il file è troppo grande
			if (mappedsize > MaxFileSize * 1024) {
				answer(fd, OP_MSG_TOOLONG);
				break;
			}
			 
			// file_name = "DirName/text"
			char* file_name = calloc(2*MaxMsgSize + 1, sizeof(char));
			strcpy(file_name, DirName);
			strcat(file_name, "/");
			strcat(file_name, text);

			int outfd = open(file_name, O_WRONLY | O_APPEND | O_CREAT, 0644);
			if (write(outfd, mappedfile, mappedsize) < 0) {
				perror("write salvando il file");
				answer(fd, OP_FAIL);
				break;
			}	
			close(outfd);
			munmap(fmsg.data.buf, fmsg.data.hdr.len);
			free(mappedfile);
			free(file_name);
			
			msg.hdr.op = FILE_MESSAGE;
			// se il ricevente è online il messaggio gli viene inviato
			if (user2->fd != -1) sendRequest(user2->fd, &msg);
			
			// in ogni caso viene inviato alla cronologia del ricevente			
			append_msg_nickname(user2, msg);
			answer(fd, OP_OK);
			update_stat(&sset, file_da_consegnare, 1);
		} break;
		
	/*---------------------GETFILE_OP--------------------------------------*/
	// gli inivio il file utilizzando mmap
		case GETFILE_OP: {			
			int reqfd = open(text, O_RDONLY);
			char* mappedfile;
			
			if (reqfd < 0) {
				perror("open");
				answer(fd, OP_NO_SUCH_FILE);
				break;
			}
			mappedfile = mmap(NULL, text_len, PROT_READ,MAP_PRIVATE, reqfd, 0);
			if (mappedfile == MAP_FAILED) {
				perror("mmap");
				answer(fd, OP_FAIL);
				break;
			}
			close(reqfd);
			
			message_t rmsg;
			rmsg.hdr.op = OP_OK;
			rmsg.data.hdr.len = text_len;
			rmsg.data.buf = mappedfile;
			
			sendRequest(fd, &rmsg);
			update_stat(&sset, file_da_consegnare, -1);
			update_stat(&sset, file_consegnati, 1);
		} break;
		
	/*------------------------GETPREVMSGS_OP------------------------------*/
		case GETPREVMSGS_OP: {
			nickname_t* user = find_hash(htab, sender);
			// se l'utente non risulta registrato
			if (user == NULL) {
				answer(fd, OP_NICK_UNKNOWN);
				break;
			}
			
			message_t rmsg;
			rmsg.hdr.op = OP_OK;
			rmsg.data.hdr.len = 1;
			rmsg.data.buf = (char*) &user->size;
			sendRequest(fd, &rmsg);
			
			int j = user->first;
			for (int i = 0; i < user->size; i++) {
				sendRequest(fd, &user->history[j]);
				j = (j + 1) % user->max_size;
			}
		} break;
		
	/*-------------------------USRLIST_OP-------------------------------*/
		case USRLIST_OP: {
			answer(fd, OP_OK);
			send_list(fd);
		} break;
	/*-----------------------UNREGISTER_OP-----------------------------*/
		case UNREGISTER_OP: {
			nickname_t* user = find_hash(htab, sender);
			if (user == NULL) {
				answer(fd, OP_NICK_UNKNOWN);
				break;
			}
			if (remove_hash(htab, sender) < 0) {
				perror("remove_hash");
				answer(fd, OP_FAIL);
				break;
			}
			
			answer(fd, OP_OK);
			update_stat(&sset, utenti_registrati, -1);
		} break;
	
	// In the following two cases worker just repost the message.	
	/*------------------TEXT_MESSAGE || FILE_MESSAGE----------------------*/
		case TXT_MESSAGE:
		case FILE_MESSAGE: {
			sendRequest(fd, &msg);
		} break;
	/*-------------------------------------------------------------------*/		
		default: break;
		}
		
		// se raggiungo questa porzione di codice allora devo comunicare
		// al listener di continuare ad ascoltare questo fd
		pthread_mutex_lock(&connected_mutex);
		connected_fd[fd] = 1;
		// scrivo sulla pipe per destare la select
		char* tmp_buf = "1";
		if (write(5, tmp_buf, 1) < 0) {
			perror("write");
			pthread_mutex_lock(&connected_mutex);
			exit(EXIT_FAILURE);
		}
		pthread_mutex_unlock(&connected_mutex);
	}
	
	return NULL;
}
