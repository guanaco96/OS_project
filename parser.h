/**
 * @file parser.h
 * @brief parser per il file di configurazione
 *
 * Si dichiara che il contenuto di questo file è in ogni sua parte
 * opera originale dell'autore.
 *
 * @author Lorenzo Beretta, 536242, loribere@gmail.com
 */
#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// lunghezza massima di una linea di testo
#define MAX_LINE_LEN 256

// variabili definite in "server.c"
extern char* UnixPath;
extern char* DirName;
extern char* StatFileName;
extern int MaxConnections;
extern int ThreadsInPool;
extern int MaxHistMsgs;
extern int MaxFileSize;
extern int MaxMsgSize;

/**
 * @brief procedura che inizializza una serie di variabili esterne
 * leggendole da un file di configuazione
 * 
 * NOTA: questa procedura non è MT-safe ma poco importa poichè viene
 * invocata solo all'avvio del server.
 * 
 * @param file_name: nome del file di configurazione
 * 
 * @return -1 se c'è errore, 0 altrimenti
 */
int parse(char* file_name) {
	FILE * fp = fopen(file_name, "r");
	if(!fp) {
		perror("errore aprendo il file di configurazione\n");
		return -1;
	}
	
	char line[MAX_LINE_LEN];
	while(!feof(fp) && fgets(line, MAX_LINE_LEN, fp)) {
		if(strlen(line) >= MAX_LINE_LEN - 1) {
			fprintf(stderr, "stringa troppo lunga nel file di configurazione\n");
			return -1;
		}
		
		if(line[0] == '#') continue;
		
		char* saveptr;
		// leggo name
		char* name = strtok_r(line, " ", &saveptr);
		name = strtok(name, "\t");
		if(!name) {
			if(feof(fp)) return 0;
			fprintf(stderr, "errore di formattazione file di configurazione\n");
			return -1;
		}
		if(name[0] == '\n') continue;
		
		// leggo eq 
		char* eq;
		eq = strtok_r(NULL, " ", &saveptr);
		if(eq) eq = strtok(eq, "\t");
		if(eq) eq = strtok(eq, "\n");
		
		if(!eq || strlen(eq) != 1 || strncmp(eq, "=", 1)) {
			fprintf(stderr, "errore di formattazione file di configurazione\n");
			return -1;
		}
		
		// leggo val
		char* val;
		do {
			val = strtok_r(NULL, " ", &saveptr);
			if(val) val = strtok(val, "\t");
			if(val) val = strtok(val, "\n");
		} while(val && !strlen(val));
		
		if(!val || strlen(val) == 0) {
			fprintf(stderr, "errore di formattazione file di configurazione\n");
			return -1;
		}
		
		// controlliamo uno ad uno tutti i casi possibili
		if(!strncmp(name, "UnixPath", strlen("UnixPath") + 1)) {
			UnixPath = calloc(strlen(val) + 1, sizeof(char));
			strncpy(UnixPath, val, strlen(val) + 1);
			continue;
		}
		if(!strncmp(name, "DirName", strlen("DirName") + 1)) {
			DirName = calloc(strlen(val) + 1, sizeof(char));
			strncpy(DirName, val, strlen(val) + 1);
			continue;
		}
		if(!strncmp(name, "StatFileName", strlen("StatFileName") + 1)) {
			StatFileName = calloc(strlen(val) + 1, sizeof(char));
			strncpy(StatFileName, val, strlen(val) + 1);
			continue;
		}
		if(!strncmp(name, "MaxConnections", strlen("MaxConnections") + 1)) {
			MaxConnections = strtol(val, NULL, 10);
			continue;
		}
		if(!strncmp(name, "ThreadsInPool", strlen("ThreadsInPool") + 1)) {
			ThreadsInPool = strtol(val, NULL, 10);
			continue;
		}	
		if(!strncmp(name, "MaxHistMsgs", strlen("MaxHistMsgs") + 1)) {
			MaxHistMsgs = strtol(val, NULL, 10);
			continue;
		}
		if(!strncmp(name, "MaxFileSize", strlen("MaxFileSize") + 1)) {
			MaxFileSize = strtol(val, NULL, 10);
			continue;
		}
		if(!strncmp(name, "MaxMsgSize", strlen("MaxMsgSize") + 1)) {
			MaxMsgSize = strtol(val, NULL, 10);
			continue;
		}
		
		// se raggiungiamo questa porzione di codice c'è un errore di formattazione
		fprintf(stderr, "errore di formattazione file di configurazione\n");
		return -1;
	}			
	
	fclose(fp);
	return 0;
}

#endif /* PARSER_H */
