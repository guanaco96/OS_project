#include <stdio.h>
#include "stat.h"

// gia inizializzato tutto a zero
stat_set_t sset;

int main() {
	update_stat(sset, clienti_online);
	char * stat_file = "stat_file";
	print_stat(sset, stat_file);
	
	return 0; 
}
