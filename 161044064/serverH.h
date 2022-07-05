#include <pthread.h>


/*handler function, just increases a flag*/
void my_handler();


/*termination when SIGINT received, learns from signalArrived flag. gives resources to the system*/
void termination();


/*handles clients. checks client wants, finds proper servant/s 
forwards request them and forward their responds requiest owners*/
void handlingClient(char* bufRead,int currentSocket);

/*init mutex and cond var for monitor*/
void initMutex();

/*sockets works. open write and read*/
int socketing(int portNum,char* bufRead, int currentSocket);

/*prints message with timestamp to STDOUT*/
void printWithTimeStamp(char* message);

/*Print usage message nicely*/
void printUsageExit(char* progName, char* message);


/*thread function*/
static void * threadFunc(void *arg);

