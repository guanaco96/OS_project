/*
 * membox Progetto del corso di LSO 2017/2018
 *
 * Dipartimento di Informatica Università di Pisa
 * Docenti: Prencipe, Torquati
 * 
 */
/**
 * @file config.h
 * @brief File contenente alcune define con valori massimi utilizzabili
 */

#if !defined(CONFIG_H_)
#define CONFIG_H_

// massima lunghezza di un nickname
#define MAX_NAME_LENGTH                  32

// massima lunghezza della coda condivisa per la gestione delle richieste dei clients
#define MAX_QUEUE_LEN 10000

// numero di caselle della tebella di hash
#define HASH_NBUCKETS 100000



// to avoid warnings like "ISO C forbids an empty translation unit"
typedef int make_iso_compilers_happy;

#endif /* CONFIG_H_ */
