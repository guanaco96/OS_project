#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "hashtable.h"
#include "queue.h"
#include "connections.h"


int main() {
	//---------------TESTING HASH_H e NICKNAME_H----------------------
	hash_t * hash_table = create_hash(10, 100);
	char* sender = "culo";
	char* reciver = "noone";
	char* text = "non ho niente da dire\n";

	insert_hash(hash_table, sender);
	insert_hash(hash_table, reciver);

	nickname_t* nk = find_hash(hash_table, sender);


	message_hdr_t mh;
	setHeader(&mh, CONNECT_OP, sender);

	message_data_t md;
	setData(&md, reciver, text, strlen(text));

	message_t m;
	memset(&m, 0, sizeof(message_t));

	m.hdr = mh;
	m.data = md;

	append_msg_nickname(nk, m);

	nk = find_hash(hash_table, sender);

	print_message(&(nk->history[nk->first]));


	//--------------------TESTING "connections.h"---------------------

	int fd = open("foo", O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);

	// usiamo il message_t m definito sopra

	sendRequest(fd, &m);

	if (close(fd) == -1) printf("errore\n");

	fd = open("foo", O_RDWR);

	message_t m2;

	readMsg(fd, &m2);

	if (close(fd) == -1) printf("errore\n");

	print_message(&m2);


	//------------free-----------------------
	destroy_hash(hash_table);

	//-------------------------------------


	return 0;
}
