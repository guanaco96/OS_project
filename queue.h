/**
 * @file queue.h
 * @brief interfaccia di una coda circolare di lunghezza fissata
 * 
 *
 * Si dichiara che il contenuto di questo file è in ogni sua parte
 * opera originale dell'autore.
 *
 * @author Lorenzo Beretta, 536242, loribere@gmail.com
 */
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#ifndef QUEUE_H
#define QUEUE_H

/**
 * @brief Rappresenta la nostra coda di lunghezza fissata
 * 
 * @var *arr: array contenente gli elementi
 * @var size: massimo numero di elementi contenibili 
 * @var len: #(elementi contenuti)
 * @var tail: posizione di estrazione
 * @var head: posizione di inserimento
 * @var mutex, cond: variabili di sincronizzazione
 */
typedef struct queue_t {
	int *arr;
	int size;
	int len;

	int tail;
	int head;
	
	pthread_mutex_t mutex;
	pthread_cond_t cond;
} queue_t;

/**
 * @brief Costruisce una nuova coda vuota
 * @param s: Massima lunghezza della coda a priori
 * @return La coda appena creata
 */
queue_t create_queue(int s);

/**
 * @brief Dealloca le strutture allocate dinamicamente nella coda
 * @param q: coda da deallocare
 */
void destroy_queue(queue_t* q);

/**
 * @brief Funzione pop MT-safe
 * @param q: Coda da cui estrarre il primo elemento
 * @return Elemento rimosso dalla coda
 */
int pop_queue(queue_t* q);

/**
 * @brief Funzione push MT-safe
 * @param q: Coda in cui inserire l'ultimo elemento
 * @param x: Elemento da inserire
 */
void push_queue(queue_t* q, int x);


#endif /* QUEUE_H */
