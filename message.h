/*
 * chatterbox Progetto del corso di LSO 2017/2018
 *
 * Dipartimento di Informatica Università di Pisa
 * Docenti: Prencipe, Torquati
 * 
 */
 /**
 * @file  message.h
 * @brief Contiene il formato del messaggio
 *
 * Si dichiara che parte del contenuto di questo file è opera originale
 * dell'autore, in particolare ho aggiunto la funzione destory_message
 * per aumentare la leggibilità del codice.
 *
 * @author Lorenzo Beretta, 536242, loribere@gmail.com
 */

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ops.h"
#include "config.h"

/**
 *  @struct header
 *  @brief header del messaggio 
 *
 *  @var op tipo di operazione richiesta al server
 *  @var sender nickname del mittente 
 */
typedef struct message_hdr_t{
    op_t     op;   
    char sender[MAX_NAME_LENGTH+1];
} message_hdr_t;

/**
 *  @struct header
 *  @brief header della parte dati
 *
 *  @var receiver nickname del ricevente
 *  @var len lunghezza del buffer dati
 */
typedef struct message_data_hdr_t {
    char receiver[MAX_NAME_LENGTH+1];
    unsigned int   len;  
} message_data_hdr_t;

/**
 *  @struct data
 *  @brief body del messaggio 
 *
 *  @var hdr header della parte dati
 *  @var buf buffer dati
 */
typedef struct message_data_t {
    message_data_hdr_t  hdr;
    char               *buf;
} message_data_t;

/**
 *  @struct messaggio
 *  @brief tipo del messaggio 
 *
 *  @var hdr header
 *  @var data dati
 */
typedef struct message_t {
    message_hdr_t  hdr;
    message_data_t data;
} message_t;

/* ------ funzioni di utilità ------- */

/**
 * @function setheader
 * @brief scrive l'header del messaggio
 *
 * @param hdr puntatore all'header
 * @param op tipo di operazione da eseguire
 * @param sender mittente del messaggio
 */
void setHeader(message_hdr_t *hdr, op_t op, char *sender);

/**
 * @function setData
 * @brief scrive la parte dati del messaggio facendo una copia del buffer
 *
 * @param msg puntatore al body del messaggio
 * @param rcv nickname o groupname del destinatario
 * @param buf puntatore al buffer 
 * @param len lunghezza del buffer
 */
void setData(message_data_t *data, char *rcv, const char *buf, unsigned int len);

/**
 * @function destroy_message
 * @brief libera la memoria allocata dinamicamente nella struttura
 *
 * @param msg puntatore al body del messaggio
 */
void destroy_message(message_t* msg);

/**
 * @brief funzione che stampa su stdout tutti i dati relativi ad un messaggio
 * 
 * Utile per il debug
 * 
 * @param msg: il messaggio da stampare
 */
void print_message(message_t* msg);

#endif /* MESSAGE_H_ */
