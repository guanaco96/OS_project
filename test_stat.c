#include <stdio.h>
#include <pthread.h>
#include "stat.h"

// gia inizializzato tutto a zero
stat_set_t sset;
char * stat_file = "stat_file";
	
int main() {
	pthread_mutex_init(&sset.mutex, NULL);
	
	update_stat(&sset, clienti_online);
	print_stat(sset, stat_file);
	
	return 0; 
}
