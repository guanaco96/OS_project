/**
 * @file connections.c
 * @brief implementazione di connections.h
 *
 * Si dichiara che il contenuto di questo file è in ogni sua parte
 * opera originale dell'autore.
 *
 * @author Lorenzo Beretta, 536242, loribere@gmail.com
 */
 
#include "connections.h"


int openConnection(char* path, unsigned int ntimes, unsigned int secs) {
	// se ntimes o secs eccedono i limiti li ridimensiono senza sollevare errori
	if(ntimes > MAX_RETRIES) ntimes = MAX_RETRIES;
	if(secs > MAX_SLEEPING) secs = MAX_SLEEPING;

	// inizializzo il socket address
	struct sockaddr_un sa;
	if(strlen(path) >= UNIX_PATH_MAX) {
		fprintf(stderr, "path troppo lungo\n");
		return -1;
	}
	strncpy(sa.sun_path, path, strlen(path) + 1);
	sa.sun_family = AF_UNIX;
	
	// creo il client socket file descriptor
	int csfd;
	if((csfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		perror("Errore creando il Socket\n");
		return -1;
	}
	
	// tento la connessione n volte
	for(int i = 0; i < ntimes; i++) {
		if(connect(csfd, (struct sockaddr*) &sa, sizeof(sa)) == 0) {
			return csfd;
		}
		sleep(secs);
	}
	
	// fallisce dopo ntimes tentativi
	perror("Errore di connessione al socket");
	return -1;
}

	
int readHeader(long fd, message_hdr_t *hdr) {
	int nred = read(fd, (void*) hdr, sizeof(message_hdr_t));
	if(nred < 0) perror("readHeader");
	if(nred == 0) fprintf(stderr, "Letto header vuoto");
	
	return nred;
}


int readData(long fd, message_data_t *data) {
	int nred = read(fd, (void*) data, sizeof(message_data_t));
	if(nred < 0) perror("readData");
	if(nred == 0) fprintf(stderr, "Letto data vuoto");
	
	return nred;
}


int readMsg(long fd, message_t *msg) {
	int nred = readHeader(fd, &(msg->hdr));
	return (nred <= 0) ? nred : readData(fd, &(msg->data));
}

int sendHeader(long fd, message_hdr_t *hdr) {
	int nwrote = write(fd, (void*) hdr, sizeof(message_hdr_t));
	if(nwrote < 0) perror("sendHeader");
	if(nwrote == 0) fprintf(stderr, "Inviato header vuoto");
	
	return nwrote;
}
	

int sendData(long fd, message_data_t *data) {
	int nwrote = write(fd, (void*) data, sizeof(message_data_t));
	if(nwrote < 0) perror("sendData");
	if(nwrote == 0) fprintf(stderr, "Inviato data vuoto");
	
	return nwrote;
}	


int sendRequest(long fd, message_t *msg) {
	int nwrote = sendHeader(fd, &(msg->hdr));
	return (nwrote <= 0) ? nwrote : sendData(fd, &(msg->data));
}



