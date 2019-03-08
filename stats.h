/**
 * @file stat.h
 * @brief Header per la definizione del tipo stat_set_t
 *
 * Si dichiara che il contenuto di questo file è in ogni sua parte
 * opera originale dell'autore.
 *
 * @author Lorenzo Beretta, 536242, loribere@gmail.com
 */

#include<stdio.h>
#include<time.h>
#include<pthread.h>

#ifndef STAT_H
#define STAT_H

// Prototipo necessario per evitare warnings dal compilatore con -std=c99
char* strtok_r(char*, const char*, char**);

/**
 * @brief associazione tra i nomi delle statistiche e degli indici
 */
typedef enum stat_t { 
	utenti_registrati = 0, 
	clienti_online = 1,
	messaggi_consegnati = 2,
	messaggi_da_consegnare = 3,
	file_consegnati = 4,
	file_da_consegnare = 5,
	messaggi_di_errore = 6
} stat_t;

/**
 * @brief struct contenente le statistiche
 * 
 * @param mutex: mutex per l'aggiornamento MT-safe delle statistiche
 */ 
typedef struct stat_set_t {
	int set[7];	
	pthread_mutex_t mutex;
} stat_set_t;

/**
 * @brief MT-safe update di una statistica
 * 
 * @param sset: set di statistiche
 * @param stat: sttistica di cui effettuare l'update
 */
void update_stat(stat_set_t* sset, stat_t stat, int increment);

/**
 * @function printStats
 * @brief Stampa le statistiche su file
 *
 * @param stat: statistiche da stampare
 * @param file_name: nome del file sul quale scrivere in append
 *
 * @return -1 in caso di errore, 0 altrimenti
 */
 
int print_stat(stat_set_t sset, const char* file_name);

#endif /* STAT_H */
