/**
 * @file hashtable.c
 * @brief implementazione di hashtable.h
 *
 * Si dichiara che il contenuto di questo file è in ogni sua parte
 * opera originale dell'autore.
 *
 * @author Lorenzo Beretta, 536242, loribere@gmail.com
 */
 
#include"hashtable.h"

hash_t* create_hash(int hsize, int nbuckets) {
	hash_t* res = malloc(sizeof(hash_t));
	if(res == NULL) {
		fprintf(stderr, "Malloc error\n");
		return NULL;
	}

	res->ht = icl_hash_create(nbuckets, NULL, NULL);
	res->history_size = hsize;
	pthread_mutex_init(&(res->mutex), NULL);
	return res;
}


int destroy_hash(hash_t* hash_table) {
	pthread_mutex_lock(&(hash_table->mutex));
	if(icl_hash_destroy(hash_table->ht, (void(*)(void*)) &free, (void(*)(void*)) &destroy_nickname) != 0) {
		fprintf(stderr, "Errore nella deallocazione della hash_table\n");
		pthread_mutex_unlock(&(hash_table->mutex));
		return -1;
	}
	pthread_mutex_unlock(&(hash_table->mutex));
	free(hash_table);
	return 0;
}


nickname_t* find_hash(hash_t* hash_table, char* key) {
	return (nickname_t*) icl_hash_find(hash_table->ht, (void*) key);
}


nickname_t* insert_hash(hash_t* hash_table, char* key) {
	pthread_mutex_lock(&(hash_table->mutex));
		
	char* key_cp = calloc(strlen(key) + 1, sizeof(char));
	strncpy(key_cp, key, strlen(key) + 1);
	if(key_cp == NULL) {
		fprintf(stderr, "Calloc error\n");
		return NULL;
	}
	nickname_t*  nk = create_nickname(hash_table->history_size);
	
	// gestione errore a livello icl_hash
	if(icl_hash_insert(hash_table->ht, (void*) key_cp, (void*) nk) == NULL) {
		fprintf(stderr, "Errore nell'inserimento in hash_table\n");
		pthread_mutex_unlock(&(hash_table->mutex));
		return NULL;	
	}
	
	pthread_mutex_unlock(&(hash_table->mutex));
	return nk;
}
	

int remove_hash(hash_t* hash_table, char* key) {
	pthread_mutex_lock(&(hash_table->mutex));
	if(icl_hash_delete(hash_table->ht, key, (void(*)(void*)) &free, (void(*)(void*)) &destroy_nickname) != 0) {
		fprintf(stderr, "Errore nell'eliminazione di %s dalla hash_table\n", key);
		pthread_mutex_unlock(&(hash_table->mutex));
		return -1;
	}
	pthread_mutex_unlock(&(hash_table->mutex));
	return 0;
}

