/**
 * @file nickname.h
 * @brief Header per la gestione del campo dato nell'hashtable dei 
 * nickname
 *
 * Si dichiara che il contenuto di questo file è in ogni sua parte
 * opera originale dell'autore.
 *
 * @author Lorenzo Beretta, 536242, loribere@gmail.com
 */
#include <pthread.h> 
#include "message.h"

#ifndef NICKNAME_H
#define NICKNAME_H

/**
 * @struct nickname
 * @brief contiene tutte l'history e il file descriptor relativi al nick
 * 
 * L'history viene mantenuta attraverso una coda circolare di lunghezza
 * massima max_size allocata dinamicamente e di cui ci salviamo l'indice
 * del primo elemento e la lunghezza totale per operarvi con le 
 * opportune operazioni aritmetiche modulo max_size.
 * 
 * @var fd: file descriptor relativo alla connessione con il client,
 * fd == -1 sse il client non è connesso.
 * 
 * @var first: indice del primo elemento della coda
 * 
 * @var size: #(elementi della coda)
 * 
 * @var max_size: massima capienza della coda
 * 
 * @var history: array contente la coda
 */
typedef struct nickname_t {
	int fd;
	int first;
	int size;
	int max_size;
	message_t* history;
	pthread_mutex_t mutex;
} nickname_t;

/**
 * @brief Inizializza il nuovo nickname impostando la capienza massima 
 * della sua history
 * 
 * @param maxs: capienza massima dell'history
 * @return nickname appena costruito, NULL in caso di errore
 */
nickname_t* create_nickname(int maxs);

/**
 * @brief delloca le strutture allocate dinamicamente di nickname
 * 
 * L'implementazione si cura di effettuare il lock affinchè la procedura
 * sia MT-safe.
 * 
 * @param nk: il nickname da liberare
 */
void destroy_nickname(nickname_t* nk);

/**
 * @brief aggiunge un messaggio in testa alla coda circolare
 * 
 * L'implementazione si cura di effettuare il lock affinchè la procedura
 * sia MT-safe.
 * 
 * @param nk: il nickname di cui fare l'update
 * @param msg: il messaggio da appendere
 */
void append_msg_nickname(nickname_t* nk, message_t msg);


#endif /* NICKNAME_H */
