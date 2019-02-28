#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>

#include "message.h"
#include "message.c"
#include "connections.h"
#include "connections.c"
	
int main() {
	int pipefd[2];
	pipe(pipefd);
	message_t smsg, rmsg;
	
	strcpy(smsg.hdr.sender, "sender");
	smsg.hdr.op = GETPREVMSGS_OP;
	smsg.data.buf = "bel messaggio";
	smsg.data.hdr.len = strlen(smsg.data.buf) + 1;
	strcpy(smsg.data.hdr.receiver, "receiver");
	
	sendRequest(pipefd[1], &smsg);
	readMsg(pipefd[0], &rmsg);
	
	print_message(&rmsg);
	
	close(pipefd[0]);
	close(pipefd[1]);
		
	return 0; 
}
