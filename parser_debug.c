#include <stdio.h>

#include "parser.h"

 char* UnixPath;
 char* DirName;
 char* StatFileName;
 int MaxConnections;
 int ThreadsInPool;
 int MaxHistMsgs;
 int MaxFileSize;
 int MaxMsgSize;


int main() {
	
	char* file_name = "DATA/chatty.conf1";
		
	if(parse(file_name) < 0) printf("errore\n");
	
	printf("%s\t%s\t%s\t%d\t%d\t%d\t%d\t%d\t",
		UnixPath,
		DirName,
		StatFileName,
		MaxConnections,
		ThreadsInPool,
		MaxHistMsgs,
		MaxFileSize,
		MaxMsgSize);
			
	
	return 0;
}
