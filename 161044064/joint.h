#ifndef _LIB_H_
#define _LIB_H_

#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include <ctype.h>
#include <sys/wait.h>
#define BACKLOG 4096

void errExit(char* str){
    fprintf(stderr,"Error: %s\n",str);
    exit(EXIT_FAILURE);
}








#endif

