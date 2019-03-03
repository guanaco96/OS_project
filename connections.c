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
	if (ntimes > MAX_RETRIES) ntimes = MAX_RETRIES;
	if (secs > MAX_SLEEPING) secs = MAX_SLEEPING;

	// inizializzo il socket address
	struct sockaddr_un sa;
	if (strlen(path) >= UNIX_PATH_MAX) {
		fprintf(stderr, "path troppo lungo\n");
		return -1;
	}
	strncpy(sa.sun_path, path, strlen(path) + 1);
	sa.sun_family = AF_UNIX;
	
	// creo il client socket file descriptor
	int csfd;
	if ((csfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		perror("Errore creando il Socket");
		return -1;
	}
	
	// tento la connessione n volte
	for (int i = 0; i < ntimes; i++) {
		if (connect(csfd, (struct sockaddr*) &sa, sizeof(sa)) == 0) {
			return csfd;
		}
		sleep(secs);
	}
	
	// fallisce dopo ntimes tentativi
	perror("Errore di connessione al socket");
	return -1;
}

/**
* @brief legge un numero fissato di bytes
*
* La lettura non viene compromessa da interruzioni in quanto
* la funzione rientra nel ciclo fintanto che tutti i bytes non
* sonon stati letti. Questo ci permette di rimediare alla non
* atomicità di della System Call read.
*
* @param fd 	il file descriptor da cui leggere
* @param buf	buffer puntante ad una regione opportunamente allocata
* @param nbyte 	numero di bytes da leggere
*
* @return 	1 se ha letto tutti gli nbyte bytes
* 			0 se il fd è chiuso
			-1 in caso di errore
*/

int safe_read(int fd, char* buf, int nbyte) {
	int nred = read(fd, buf, nbyte);
	while (nbyte > 0) {
		if (nred < 0 && errno != EINTR) {
			return -1;
		}
		if (nred == 0) return 0;
		
		buf += nred;
		nbyte -= nred;
	}
	return 1;
}

/**
* @brief scrive un numero fissato di bytes
*
* La scrittura non viene compromessa da interruzioni in quanto
* la funzione rientra nel ciclo fintanto che tutti i bytes non
* sonon stati scritti. Questo ci permette di rimediare alla non
* atomicità di della System Call write.
*
* @param fd 	il file descriptor su cui scrivere
* @param buf	buffer puntante la regione da scrivere sul fd
* @param nbyte 	numero di bytes da scrivere
*
* @return 	1 se ha scritto tutti gli nbyte bytes
* 			0 se il fd è chiuso
			-1 in caso di errore
*/

int safe_write(int fd, char* buf, int nbyte) {
	int nwritten = write(fd, buf, nbyte);
	while (nbyte > 0) {
		if (nwritten < 0 && errno != EINTR) {
			return -1;
		}
		if (nwritten == 0) return 0;
		
		buf += nwritten;
		nbyte -= nwritten;
	}
	return 1;
}
	
int readHeader(long fd, message_hdr_t *hdr) {
	return safe_read(fd, (char*) hdr, sizeof(message_hdr_t));
}


int readData(long fd, message_data_t *data) {
	int res = safe_read(fd, (char*) &data->hdr, sizeof(message_data_hdr_t));
	
	data->buf = (data->hdr.len > 0) ? calloc(data->hdr.len, sizeof(char)) : NULL;
	
	return (res <= 0) ? res : safe_read(fd, data->buf, data->hdr.len); 
}


int readMsg(long fd, message_t *msg) {
	int nred = readHeader(fd, &(msg->hdr));
	return (nred <= 0) ? nred : readData(fd, &(msg->data));
}

int sendHeader(long fd, message_hdr_t *hdr) {
	return safe_write(fd, (char*) hdr, sizeof(message_hdr_t));
}
	

int sendData(long fd, message_data_t *data) {
	int res = safe_write(fd, (char*) &data->hdr, sizeof(message_data_hdr_t));
	return (res <= 0) ? res : safe_write(fd, data->buf, data->hdr.len); 
}


int sendRequest(long fd, message_t *msg) {
	int nwritten = sendHeader(fd, &(msg->hdr));
	return (nwritten <= 0) ? nwritten : sendData(fd, &(msg->data));
}



