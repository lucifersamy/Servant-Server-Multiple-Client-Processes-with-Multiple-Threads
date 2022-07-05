#include <time.h>
#include <sys/time.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>

/*handler function, just increases a flag*/
void my_handler();


/*termination when SIGINT received, learns from signalArrived flag. gives resources to the system*/
void termination();

/*prints usage information*/
void printUsageExit(char* progName, char* message);

/*loads dataset from disk*/
void loadDataFromDisk(char *path, int start, int end, int flag);

/*Returns the PID, from the /proc/self symlink.*/
pid_t getPidFromProc();

/*get host ip for sending to server*/
char* getip();

/*sets uniq portnum using a file. protect with a semaphore*/
void setPortNum();


/*thread function*/
static void * threadFunc(void *arg);