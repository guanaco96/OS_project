/**
 * @file hashtable.h
 * @brief hashtable per lo storage dei nicknames
 *
 * Si dichiara che il contenuto di questo file è in ogni sua parte
 * opera originale dell'autore.
 *
 * @author Lorenzo Beretta, 536242, loribere@gmail.com
 */

#ifndef HASH_H
#define HASH_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "icl_hash.h"
#include "nickname.h"

/**
 * @struct hash_t
 * @brief struttura che specializza la icl_hash_t fornendola di un mutex
 * 	e un limite uniforme sulla size della history per i nickname in essa
 * 	contenuti
 *
 * Le chiavi sono i nickname e i dati sono del tipo nickname_t, i nomi
 * contenuti nell'hash table sono tutti e soli quelli degli utenti
 * registrati. Per ottenere un buon fattore di carico sarà opportuno
 * proporzionare il numero di buckets al numero previsto di utenti registrati
 * che nel nostro caso è 10^5.
 * 
 * @var ht: hash table generica tratta da "icl_hash.h"
 * @var history_size: la size massima dell'history per tutti gli utenti
 * @var mutex: per il locking dei dati condivisi
 */
typedef struct hash_t {
	icl_hash_t* ht;
	int history_size;
	pthread_mutex_t mutex;
} hash_t;

/**
 * @brief crea la struttura dati
 * 
 * @param hsize: size dell'history
 * @param nbuckets: numero di buckets della tabella
 */
hash_t* create_hash(int hsize, int nbuckets);

/**
 * @brief dealloca la memoria allocata dinamicamente nella struttura e
 * la memoria puntata dall'argomento
 * 
 * @param hash_table: la struttura di cui deallocare le parti dinamiche
 * @return 0 se ha successo, -1 se da errore
 */
int destroy_hash(hash_t* hash_table);

/**
 * @brief trova il dato associato ad una chiave in O(1)
 * 
 * @param hash_table: la tabella in cui cercare
 * @param key: chiave che si sta cercando
 * 
 * @return il dato di tipo nickname_t associato, 
 * se key non si trova è restituito NULL
 */
nickname_t* find_hash(hash_t* hash_table, char* key);

/**
 * @brief inserisce una nuova chiave nella tabella associandogli un
 *	una history vuota
 * 
 * @param hash_table: la tabella in cui effettuare l'inserimento
 * @param key: la chiave da inserire, nonchè il nome del dato nickname_t
 * 
 * @return il dato associato a key, NULL se si ha errore.
 */ 
nickname_t* insert_hash(hash_t* hash_table, char* key);

/**
 * @brief rimuove la coppia (key, dato(key)) dalla struttura
 * 
 * @param hash_table: sturttura da cui eliminare i dati
 * @param key: chiave da eliminare
 * 
 * @return 0 se ha successo, -1 se fallisce
 */
int remove_hash(hash_t* hash_table, char* key);


#endif /* HASH_H */
