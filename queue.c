/**
 * @file queue.c
 * @brief Implementazione di queue.h
 *
 * Si dichiara che il contenuto di questo file è in ogni sua parte
 * opera originale dell'autore.
 *
 * @author 	Lorenzo Beretta, 536242, loribere@gmail.com
 */
#include "queue.h"

// Funzioni esterne (documentate in "queue.h")

queue_t create_queue(int s) {
	queue_t res;
	res.size = s;
	res.arr = calloc(s, sizeof(int));
	res.len = res.tail = res.head = 0;
	
	pthread_mutex_init(&(res.mutex), NULL);
	pthread_cond_init(&(res.cond), NULL);
	return res;
}

void destroy_queue(queue_t* q) {
	free(q->arr);
	pthread_mutex_destroy(&(q->mutex));
}

int pop_queue(queue_t* q) {
	pthread_mutex_lock(&(q->mutex));
	while (q->len == 0)
		pthread_cond_wait(&(q->cond), &(q->mutex));
	int res = q->arr[q->tail];
	q->tail = (q->tail + 1) % q->size;
	q->len--;
	pthread_mutex_unlock(&(q->mutex));
	return res;
}

void push_queue(queue_t* q, int x) {
	pthread_mutex_lock(&(q->mutex));
	if (q->len == q->size) {
		fprintf(stderr, "Queue piena\n");
		exit(EXIT_FAILURE);
	}
	q->arr[q->head] = x;
	q->len++;
	q->head = (q->head + 1) % q->size;
	pthread_cond_signal(&(q->cond));
	pthread_mutex_unlock(&(q->mutex));
}
