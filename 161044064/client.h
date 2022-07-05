#include <pthread.h>

/*thread function*/
static void * threadFunc(void *arg);

/*prints usage information*/
void printUsageExit(char* progName, char* message);

/*mutex and cond var init*/
void initMutex();

/*load requirements data from file*/
void loadRequestsFromFile(char* requestFile);

/*clear unnecessary spaces*/
char* clearSpaces(char* line);