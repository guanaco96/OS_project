/**
 * @file nickname.c
 * @brief Implementazione di nickname.h
 *
 * Si dichiara che il contenuto di questo file è in ogni sua parte
 * opera originale dell'autore.
 *
 * @author 	Lorenzo Beretta, 536242, loribere@gmail.com
 */
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "nickname.h"

// Funzioni esterne (documentate in "nickname.h")

nickname_t* create_nickname(int maxs) {
	assert(maxs > 0);
	nickname_t * res = malloc(sizeof(nickname_t));
	res->fd = -1;
	res->first = res->size = 0;
	res->max_size = maxs;
	if((res->history = calloc(maxs, sizeof(message_t))) == NULL) {
		fprintf(stderr, "Errore calloc\n");
		return NULL;
	}
	if(pthread_mutex_init(&(res->mutex), NULL) != 0) {
		fprintf(stderr, "Errore inizializzaizone mutex\n");
		return NULL;
	}
	return res;
}

void destroy_nickname(nickname_t* nk) {
	pthread_mutex_lock(&(nk->mutex));
	for(int i = nk->first, j = 0; j < nk->size; j++) {
		destroy_message(&(nk->history[i]));
		i = (i + 1) % nk->max_size;
	}
	free(nk->history);
	pthread_mutex_unlock(&(nk->mutex));
	free(nk);
}	

void append_msg_nickname(nickname_t* nk, message_t msg) {
	pthread_mutex_lock(&(nk->mutex));
	if(nk->size == nk->max_size) {
		destroy_message(&(nk->history[nk->first]));
		nk->history[nk->first] = msg;
		nk->first = (nk->first + 1) % nk->max_size;
	}
	else {
		int head = (nk->first + nk->size) % nk->max_size;
		nk->history[head] = msg;
		nk->size++;
	}
	pthread_mutex_unlock(&(nk->mutex));
}		
