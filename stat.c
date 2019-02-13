/**
 * @file stat.c
 * @brief implementazione di stat.h
 *
 * Si dichiara che il contenuto di questo file è in ogni sua parte
 * opera originale dell'autore.
 *
 * @author Lorenzo Beretta, 536242, loribere@gmail.com
 */
 
#include"stat.h"

void update_stat(stat_set_t* sset, stat_t stat) {
	pthread_mutex_lock(&(sset->mutex));
	sset->set[(int) stat]++;
	pthread_mutex_unlock(&(sset->mutex));
}

 
int print_stat(stat_set_t sset, const char* file_name) {
    FILE * fp = fopen(file_name, "a");
    if(fp == NULL) {
		perror("fopen");
		return -1;
	}
	
    if(fprintf(fp, "%d -",	(int)time(NULL)) < 0) {
			fprintf(stderr, "stampa stat fallita\n");
			return -1;
    }
    
    pthread_mutex_lock(&(sset.mutex));    
    for(int i = 0; i < 7; i++) {
		if(fprintf(fp, " %d", sset.set[i]) < 0) {
			fprintf(stderr, "stampa stat fallita\n");
			return -1;
		}
	}
    pthread_mutex_unlock(&(sset.mutex));
	
    if(fprintf(fp, "\n") < 0) {
		fprintf(stderr, "stampa stat fallita\n");
		return -1;
    }
    
    return 0;
}
