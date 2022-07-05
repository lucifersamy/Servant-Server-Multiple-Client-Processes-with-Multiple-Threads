#include "network.h"

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <fcntl.h> 
#include <unistd.h> 


int openSocket(int socket1,int PORT,char* IP){
	struct sockaddr_in serverAddr;

    if((socket1 = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    serverAddr.sin_family = AF_INET; 
    serverAddr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, IP, &serverAddr.sin_addr)<=0)  
    {
    	perror("inet pton");
        exit(EXIT_FAILURE);
    }

    if(connect(socket1, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) 
    {   
        close(socket1);
        perror("connect");
        exit(EXIT_FAILURE);
    }
    
    return socket1;
}