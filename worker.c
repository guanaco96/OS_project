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

// variabili esterne documentate in server.c
extern stat_set_t sset;
extern queue_t queue;
extern hash_t* htab;
extern char loop_interrupt;
extern int* connected_fd;
extern pthread_mutex_t connected_mutex;
extern char** fd_to_nick;

// se il client si è disconnesso lo elimino dalla mappa fd_to_nick
// altrimenti segnalo al listener che deve essere riascoltato 
void is_connected(int fd) {
	pthread_mutex_lock(&connected_mutex);
	
	if(fcntl(fd, F_GETFD) != -1) {
		connected_fd[fd] = 1;
		// scrivo sulla pipe per destare la select
		write(5, "1", 1);
	}
	else {
		// elimino il fd dalla hash table
		nickname_t* usr = find_hash(htab, fd_to_nick[fd]);
		if(usr == NULL) {
			fprintf(stderr, "Errore find hash table\n");
			exit(EXIT_FAILURE);
		} 
		usr->fd = -1;
		// elimino il nickname dalla mappa inversa
		fd_to_nick[fd] = NULL;
	}
	
	pthread_mutex_unlock(&connected_mutex);
}

// aggiuinge un nickname alla mappa fd_to_nickname
void add_nick(int fd, char* nick) {
	pthread_mutex_lock(&connected_mutex);
	fd_to_nick[fd] = nick;
	pthread_mutex_unlock(&connected_mutex);
}

// invia alla connesione data da fd la lista dei client connessi formattata
// in accordo con il file client.c
void send_list(int fd) {
	message_data_t data;
	data.buf = calloc(MaxConnections * (MAX_NAME_LENGTH + 1), sizeof(char));
	data.hdr.len = 0;
	char* ptr = data.buf;
	
	pthread_mutex_lock(&connected_mutex);
	for(int j = 6; j < MaxConnections + 6; j++) {
		if(fd_to_nick[j]) {
			strncpy(ptr, fd_to_nick[j], MAX_NAME_LENGTH + 1);
			ptr += MAX_NAME_LENGTH + 1;
			data.hdr.len += MAX_NAME_LENGTH + 1;
		}
	}	
	pthread_mutex_unlock(&connected_mutex);
	
	sendData(fd, &data);
}

// invia una risposta al client contenente solo l'operazione
void answer(int fd, op_t op) {
	message_hdr_t reply;
	reply.op = op;
	sendHeader(fd, &reply);
}
	

void* worker(void* useless_arg) {
	message_t msg;
	memset(&msg, 0, sizeof(message_t));
	
	while(!loop_interrupt) {
		// estrazione MT-safe dalla queue
		int fd = pop_queue(&queue);
				
		// lettura del messaggio
		readMsg(fd, &msg);
		op_t op = msg.hdr.op;
		char* sender = msg.hdr.sender;
		char* receiver = msg.data.hdr.receiver;
		char* text = msg.data.buf;
		int text_len = msg.data.hdr.len;
		
		// eseguo l'operazione
		switch(op) {
	/*------------------------REGISTER_OP---------------------------------*/
		case REGISTER_OP: {
			// caso in cui il nick è già registrato
			if(find_hash(htab, sender) != NULL) {
				answer(fd, OP_NICK_ALREADY);
				break;
			}
			nickname_t* user = insert_hash(htab, sender);
			// se l'inserimento nella table fallisce
			if(user == NULL) {
				fprintf(stderr, "Errore inserimento hash table\n");
				answer(fd, OP_FAIL);
				break;
			}
			// se va tutto a buon fine
			answer(fd, OP_OK);
			user->fd = fd;
			add_nick(fd, sender);
			send_list(fd);
		} break;
	/*---------------------------CONNECT_OP------------------------------*/
		case CONNECT_OP: {
			nickname_t* user = find_hash(htab, sender);
			// se l'utente non risulta registrato
			if(user == NULL) {
				answer(fd, OP_NICK_UNKNOWN);
				break;
			}
			// se va tutto a buon fine
			answer(fd, OP_OK);
			user->fd = fd;
			add_nick(fd, sender);
			send_list(fd);			
		} break;
	/*-----------------------------POSTTXT_OP-----------------------------*/
		case POSTTXT_OP: {
			nickname_t* user1 = find_hash(htab, sender);
			nickname_t* user2 = find_hash(htab, receiver);
			// se uno dei nicknames non è riconosciuto
			if(!user1 || !user2) {
				answer(fd, OP_NICK_UNKNOWN);
				break;
			}
			
			msg.hdr.op = TXT_MESSAGE;
			
			if(user2->fd != -1) sendRequest(user2->fd, &msg);

			append_msg_nickname(user1, msg);
			append_msg_nickname(user2, msg);
			anwer(fd, OP_OK);
		} break;
	/*-----------------------------POSTTXTALL_OP--------------------------*/
		case POSTTXTALL_OP: {
			nickname_t* user = find_hash(htab, sender);
			
			if(user == NULL) {
				answer(fd, OP_NICK_UNKNOWN);
				break;
			}
			
			pthread_mutex_lock(&connected_mutex);
			msg.hdr.op = TXT_MESSAGE;
			for(int i = 6; i < MaxConnections + 6; i++) {
				if(fd_to_nick[i] == NULL) continue;
				sendRequest(i, &msg);
			}
			
			
			
			
			
			
			
			
			
			
		
			
	
	
	
	return NULL;
}
