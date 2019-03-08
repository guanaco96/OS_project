/**
 * @file stat.c
 * @brief implementazione di stat.h
 *
 * Si dichiara che il contenuto di questo file è in ogni sua parte
 * opera originale dell'autore.
 *
 * @author Lorenzo Beretta, 536242, loribere@gmail.com
 */
 
#include"stats.h"

void update_stat(stat_set_t* sset, stat_t stat, int increment) {
	pthread_mutex_lock(&(sset->mutex));
	sset->set[(int) stat] += increment;
	pthread_mutex_unlock(&(sset->mutex));
}

 
int print_stat(stat_set_t sset, const char* file_name) {
    pthread_mutex_lock(&(sset.mutex));
    FILE * fp = fopen(file_name, "a");
    if (fp == NULL) {
		perror("fopen");
		return -1;
	}
	
    if (fprintf(fp, "%ld -", time(NULL)) < 0) {
			fprintf(stderr, "stampa stat fallita\n");
			return -1;
    }
    
    for (int i = 0; i < 7; i++) {
		if (fprintf(fp, " %d", sset.set[i]) < 0) {
			fprintf(stderr, "stampa stat fallita\n");
			return -1;
		}
	}
 	
    if (fprintf(fp, "\n") < 0) {
		fprintf(stderr, "stampa stat fallita\n");
		return -1;
    }
    fflush(fp);
    fclose(fp);
    pthread_mutex_unlock(&(sset.mutex));
    return 0;
}
