#include "joint.h"
#include "client.h"
#include "network.h"

/*2d str for all requirements from file*/
char** datas = NULL;

/*mutex, PTHREAD_MUTEX_NORMAL mutex attributes and condition var for synchronization barrier.*/
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutexattr_t mtxAttr;
pthread_mutex_t mtx;

/*keeping thread ids*/
pthread_t* thread_id = NULL;


int socket1 = -1;
int PORT;
char* IP;
int arrived=0;
int totalReqNum=0; //basically thread num


int main(int argc, char *argv[])
{
    int opt;
    char* requestFile=NULL;

    while((opt = getopt(argc, argv, ":r:q:s:")) != -1)  
    {  
        switch(opt)  
        {  
            case 'r':
                requestFile = optarg;
                break;
            case 'q':
                PORT = atoi(optarg);
                break;
            case 's':
                IP = optarg;
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

    if(optind!=7){
        errno=EINVAL;
        printUsageExit(argv[0],"Missing argument"); 
    }

    if(PORT < 0)
    {
        printUsageExit(argv[0], "Port number must be bigger than 0, preferable >2000");
    }


    initMutex();
    loadRequestsFromFile(requestFile);


    thread_id = (pthread_t*) calloc(totalReqNum, sizeof(pthread_t));
    int threadNums[totalReqNum];

    int returnValue;
    int i;
    for(i=0; i<totalReqNum; ++i){
        threadNums[i] = i;
        returnValue = pthread_create(&thread_id[i], NULL, threadFunc, &threadNums[i]);
        if (returnValue != 0){
            fprintf(stderr, "pthread_create supplier\n");
            exit(EXIT_FAILURE); 
        }
    }


    for(i=0; i<totalReqNum; ++i){
        pthread_join(thread_id[i],NULL);
    }

    printf("Client: All threads have terminated, goodbye.\n");
    free(thread_id);
    exit(EXIT_SUCCESS);

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

void loadRequestsFromFile(char* requestFile){
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int i;

    fp = fopen(requestFile, "r");
    if (fp == NULL){
        perror("Open file");
        exit(EXIT_FAILURE);
    }

    int maxStr=0;

    while ((read = getline(&line, &len, fp)) != -1) {
        totalReqNum++;
        if(strlen(line)>maxStr){
            maxStr = strlen(line)+5;
        }
    }

    fclose(fp);
    

    fp = fopen(requestFile, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    datas = calloc(totalReqNum, sizeof *datas);
    for (i=0; i<maxStr; i++){
        datas[i] = calloc(maxStr, sizeof *(datas[i]));
    }
    int realRequestNum=0;
    size_t  lineSize=0;
    for(i=0;i<totalReqNum;++i){
        getline(&line, &lineSize, fp);
        if((line=clearSpaces(line)) != NULL){
            strcpy(datas[i], line);
            realRequestNum++;
        }
    }
    totalReqNum = realRequestNum;

    if (line)
        free(line);
    fclose(fp);
    printf("Client: I have loaded %d requests and I’m creating %d threads.\n", totalReqNum, totalReqNum);
}




char* clearSpaces(char* line){
    int i = 0;
    char tempLine[strlen(line)];
    int wordStart=0;
    int i_tempLine=0;
    int flag=1;
    int beginning=0;

    while(i<strlen(line) && flag){
        if(line[i] != ' '){
            if(line[i] != '\n'  && line[i] != EOF){
                wordStart = 1;
            }
            beginning = 1;
            tempLine[i_tempLine] = line[i];
            i_tempLine++;
            i++;
        }
        else{ //boşluksa
            while(line[i] == ' '){
                if(line[i+1] == '\n'){
                    tempLine[i_tempLine] = line[i+1];
                    flag = 0;
                    break;
                }
                else if(line[i+1] != ' ' && beginning){
                    tempLine[i_tempLine] = line[i];
                    i_tempLine++;
                    ++i;
                    break;
                }
                ++i;
            }
        }
        
    }

    if(wordStart == 0){
        return NULL;
    }
    memset(line, 0, strlen(line));
    i=0;
    while(tempLine[i] != '\n' && tempLine[i] != EOF){
        line[i] = tempLine[i];
        ++i;
    }
    
    line[i] = '\n';
    line[i+1] = '\0';
   
    return line;
}
static void * threadFunc(void *arg)
{
    int requestNum = *(int*) arg;
    printf("Client-Thread-%d: Thread-%d has been created\n", requestNum, requestNum);
    int socketD;

     //synchronization barrier
    pthread_mutex_lock(&mtx);
    ++arrived;

    while(arrived<totalReqNum){
        pthread_cond_wait(&cond,&mtx);
    }
    
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mtx);

    pthread_mutex_destroy(&mtx);
    pthread_cond_destroy(&cond);

    char bufRead[100];
    char bufWrite[100];

    bzero(bufRead,sizeof(bufRead));
    bzero(bufWrite,sizeof(bufWrite));

    char printStr[strlen(datas[requestNum])];
    strcpy(printStr,datas[requestNum]);
    printStr[strlen(printStr)-1] = '\0';

    printf("Client-Thread-%d: I am requesting \"/%s\"\n",requestNum, printStr);
    
    socketD = openSocket(socket1,PORT, IP);

    send(socketD,  datas[requestNum], strlen( datas[requestNum]), 0);

    read(socketD, bufRead, sizeof(bufRead));

    
    if(bufRead[0] == 'E')
        printf("Error\n");
    else
        printf("Client-Thread-%d: The server’s response to \"/%s\" is %s\n",requestNum, printStr, bufRead);

    close(socketD);

    printf("Client-Thread-%d: Terminating\n",requestNum );

    return 0;

}


void printUsageExit(char* progName, char* message){
    fprintf(stderr, "%s\n",message );
    fprintf(stderr, "Usage: %s -r requestFile -q PORT -s IP\n", progName);
    exit(EXIT_FAILURE);
}