#include "serverH.h"
#include "joint.h"
#include "Queue.h"
#include "network.h"

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutexattr_t mtxAttr;
pthread_mutex_t mtx;


int signalArrived=0;


int numberOfThreads;
Queue sockets;
char** arr2d;
int row=1;
int socket1 = -1;
char servantIP[15];
pthread_t* thread_id = NULL;
int totalHandledRequest=0;

void my_handler(){
    signalArrived++;
}


int main(int argc, char *argv[]){

    int opt;
    int PORT;

    while((opt = getopt(argc, argv, ":p:t:")) != -1)  
    {  
        switch(opt)  
        {  
            case 't':
                numberOfThreads = atoi(optarg);
                break;
            case 'p':
                PORT = atoi(optarg);
                break;
            case ':':  
                errno=EINVAL;
                printUsageExit(argv[0],"Option needs a value.");
                break;  
            case '?':  
                errno=EINVAL;
                printUsageExit(argv[0],"Unknown");
                break; 
            case -1:
                break;
            default:
                abort (); 
        }
    }

    if(optind!=5){
        errno=EINVAL;
        printUsageExit(argv[0],"Missing argument"); 
        exit(EXIT_FAILURE); 
    }

    if(PORT<0)
    {
        printUsageExit(argv[0], "Port number must be bigger than 0, preferable >2000");
    }
    if(numberOfThreads<5){
        printUsageExit(argv[0], "Number of threads must be bigger than 5");
    }

    struct sigaction newact;
    newact.sa_handler = &my_handler;
    newact.sa_flags = 0;

    if((sigemptyset(&newact.sa_mask) == -1) || (sigaction(SIGINT, &newact, NULL) == -1) ){
        perror("Failled to install SIGINT signal handler");
        exit(EXIT_FAILURE);
    }
    
    
    sockets.front = NULL;
    sockets.rear = NULL;

    struct sockaddr_in serverAddr;
    int socketD;
    int addrlen = sizeof(serverAddr);
    int opt1 = 1;

    if(signalArrived != 0){
        termination();
    }

    if((socket1 = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    {
        errExit("socket");
    } 

    if(setsockopt(socket1, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt1, sizeof(opt1))) 
    {
        errExit("setsockopt");
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; 
    serverAddr.sin_port = htons(PORT);

    if(bind(socket1, (struct sockaddr *)&serverAddr, sizeof(serverAddr))<0) 
    {
        errExit("bind %s");
    }

    if (listen(socket1, BACKLOG) < 0) 
    {
        errExit("listen");
        exit(EXIT_FAILURE); 
    }


    thread_id = (pthread_t*) calloc(numberOfThreads, sizeof(pthread_t));
    int threadNums[numberOfThreads];

    int returnValue,i;

    for(i=0; i<numberOfThreads; ++i){
        threadNums[i] = i;
        returnValue = pthread_create(&thread_id[i], NULL, threadFunc, &threadNums[i]);
        if (returnValue != 0){
            fprintf(stderr, "pthread_create supplier\n");
            exit(EXIT_FAILURE); 
        }
    }

    initMutex();

    int y, col=1000;
    arr2d = (char **) malloc(sizeof(char *) * 1000); 
    for (y = 0; y < 1000; y++) {
        arr2d[y] = malloc(sizeof(char) * col); 
    }


    while(signalArrived == 0)
    {
        if(signalArrived == 0 && (socketD = accept(socket1, (struct sockaddr *)&serverAddr, (socklen_t*)&addrlen))<0)
        {
            if(signalArrived != 0){
                termination();
            }
        }

        pthread_mutex_lock(&mtx);
        enqueue(&sockets, socketD);
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mtx);
        
    }


    for(i=0; i<numberOfThreads; ++i){
        pthread_join(thread_id[i],NULL);
    }


    free(thread_id);

}

static void * threadFunc(void *arg)
{
    int y;
    int currentSocket;
    char bufRead[1000];
    for(;;){
        pthread_mutex_lock(&mtx);

        while(isEmpty(&sockets) == 0){
            pthread_cond_wait(&cond,&mtx);
        }

        currentSocket = dequeue(&sockets);
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mtx);

        bzero(bufRead,sizeof(bufRead));
        read(currentSocket, bufRead, sizeof(bufRead));

        if(bufRead[0] == 'S'){
            if(row == 1000){
                arr2d = realloc(arr2d, sizeof(char *) * 2000);
                for (y = 0; y < 2000; y++) {
                    arr2d[y] = realloc(arr2d[y], sizeof(char) * 1000);
                    if (arr2d[y] == NULL) {
                        perror("realloc");
                        exit(EXIT_FAILURE);
                    }
                }
            }

            strcpy(arr2d[row-1],bufRead);
            
            row++;
            close(currentSocket);
        }

        
        if(bufRead[0] == 't'){
            handlingClient(bufRead,currentSocket);
        }

    }

   return 0;
}



void initMutex(){
    int s;
    s = pthread_mutexattr_init(&mtxAttr);
    if (s != 0){
        perror("pthread_mutexattr_init");
        exit(EXIT_FAILURE);
    }
    s = pthread_mutexattr_settype(&mtxAttr, PTHREAD_MUTEX_NORMAL);
    if (s != 0){
        perror("pthread_mutexattr_settype");
        exit(EXIT_FAILURE);
    }
    s = pthread_mutex_init(&mtx, &mtxAttr);
    if (s != 0){
        perror("pthread_mutexattr_init");
        exit(EXIT_FAILURE);
    }
    s = pthread_mutexattr_destroy(&mtxAttr); 
    if (s != 0){
        perror("pthread_mutexattr_destroy");
        exit(EXIT_FAILURE);
    }
}

void printWithTimeStamp(char* message){
    time_t     now;
    struct tm  ts;
    char       buf[40];
    time(&now);
    ts = *localtime(&now);
    strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", &ts);
    printf("[%s]\t%s",buf,message);
}

void handlingClient(char* bufRead, int currentSocket){
    char city[40], type[50],d1[12], d2[12];
    bzero(city,sizeof(city));

    int portNum;
    int totalTransactionResult = 0, i=0;
    char trash2[1000];
    int isFounded=0,pid;

    char printStr[strlen(bufRead)];
    strcpy(printStr,bufRead);
    printStr[strlen(printStr)-1] = '\0';

    char* message1 = calloc(400, sizeof *message1);
    sprintf(message1,"Request arrived \"%s\"\n",printStr);
    printWithTimeStamp(message1);
    free(message1);
    
    if(isalpha(bufRead[strlen(bufRead)-2]) == 0){
        message1 = calloc(400, sizeof *message1);
        sprintf(message1,"Contacting ALL servants\n");
        printWithTimeStamp(message1);
        free(message1);

        isFounded = 1;
        sscanf(bufRead,"transactionCount %s %s %s\n",type, d1, d2);
        for(i=0; i<row-1; ++i){
            portNum = 0;
            sscanf(arr2d[i], "S%d %d %s %s", &portNum, &pid, servantIP, trash2);
            totalTransactionResult += socketing(portNum,bufRead,currentSocket);
        }
    }
    else{
        sscanf(bufRead,"transactionCount %s %s %s %s\n",type, d1, d2,city);
        for(i=0; i<row-1; ++i){
            portNum = 0;
            if(strstr(arr2d[i], city) != NULL){
                isFounded = 1;
                sscanf(arr2d[i], "S%d %d %s %s", &portNum, &pid, servantIP, trash2);

                message1 = calloc(400, sizeof *message1);
                sprintf(message1,"Contacting servant %d\n",pid);
                printWithTimeStamp(message1);

                totalTransactionResult += socketing(portNum,bufRead,currentSocket);
                break;
            }
        }
    }
    char strTotalTransactionResult[30];
    if(isFounded){
        sprintf(strTotalTransactionResult, "%d", totalTransactionResult);

        message1 = calloc(400, sizeof *message1);
        sprintf(message1,"Response received: %d, forwarded to client\n",totalTransactionResult);
        printWithTimeStamp(message1);
    }
    else{
        strcpy(strTotalTransactionResult, "ERROR");
    }

    write(currentSocket, strTotalTransactionResult, strlen(strTotalTransactionResult));
    totalHandledRequest++;
    close(currentSocket);

}


int socketing(int portNum,char* bufRead, int currentSocket){
    int socket = -1;

    socket = openSocket(socket, portNum, servantIP);
    
    send(socket,  bufRead, strlen(bufRead), 0);

    char newBUF[1000];
    bzero(newBUF,sizeof(newBUF));
    read(socket, newBUF, sizeof(newBUF));

    close(socket);

    return atoi(newBUF);
}

void termination(){

    char* message1 = calloc(400, sizeof *message1);
    sprintf(message1,"SIGINT has been received. I handled a total of %d requests. Goodbye.\n",totalHandledRequest);
    printWithTimeStamp(message1);
    free(message1);

    if(socket1 != -1){
        close(socket1);
    }
    
    if(thread_id != NULL){
        free(thread_id);
    }

    int pid,portNum;
    char trash[1000];
    
    for(int i=0; i<row-1; ++i){
        sscanf(arr2d[i], "S%d %d %s", &portNum, &pid, trash);
        kill(pid, SIGINT);
    }


    exit(EXIT_SUCCESS);
}

void printUsageExit(char* progName,char* message){
    fprintf(stderr, "%s\n",message );
    fprintf(stderr, "Usage: %s -d directoryPath -c 10-19 -r IP -p PORT\n", progName);
    exit(EXIT_FAILURE);
}
