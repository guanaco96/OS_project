/**
 * @file  message.c
 * @brief Implementazione di message.h
 *
 * Si dichiara che parte del contenuto di questo file è opera originale
 * dell'autore, in particolare ho aggiunto la funzione destory_message
 * per aumentare la leggibilità del codice.
 *
 * @author Lorenzo Beretta, 536242, loribere@gmail.com
 */

#include "message.h"

// La documentazione delle funzioni si trova in "message.h"

void setHeader(message_hdr_t *hdr, op_t op, char *sender) {
#if defined(MAKE_VALGRIND_HAPPY)
    memset((char*)hdr, 0, sizeof(message_hdr_t));
#endif 
    hdr->op  = op;
    strncpy(hdr->sender, sender, strlen(sender)+1);
}

void setData(message_data_t *data, char *rcv, const char *buf, unsigned int len) {
#if defined(MAKE_VALGRIND_HAPPY)
    memset((char*)&(data->hdr), 0, sizeof(message_data_hdr_t));
#endif 

    strncpy(data->hdr.receiver, rcv, strlen(rcv)+1);
    data->hdr.len  = len;
    if (buf) {
		data->buf = calloc(strlen(buf) + 1, sizeof(char));
		strncpy(data->buf, buf, strlen(buf) + 1);
	} else {
		data->buf = NULL;
	}
}

void destroy_message(message_t* msg) {
	free(msg->data.buf);
}

void print_message(message_t* msg) {
	printf("sender = %s\n", msg->hdr.sender);
	printf("operation = %d\n", (int)msg->hdr.op);
	printf("receiver = %s\n", msg->data.hdr.receiver);
	printf("text = %s\n", msg->data.buf);
}
