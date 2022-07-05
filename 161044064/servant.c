#include "joint.h"
#include "servantH.h"
#include "LinkedList.h"

#include "network.h"


#define semapName "/semapSem1"
#define FILE_NAME "port_num_file.txt"


sem_t* semap;

Transaction* newTransaction;
Date* newDate;
City* newCity;
Country* country;


char bufWrite[2000],bufRead[2000];

int socket1 = -1;
int socketD= -1;
int portNum;
int signalArrived = 0;
int retValue;

void my_handler(){
    signalArrived++;
}

pthread_t* thread_id = NULL;
int* socketNums = NULL;
char startCity[200], endCity[200];
int totalHandledRequest=0;


int main(int argc, char *argv[])
{
    int opt;
    char* directoryPath=NULL;
    char* cityNumbers=NULL;
    int PORT;
    char* IP;

    while((opt = getopt(argc, argv, ":d:c:r:p:")) != -1)  
    {  
        switch(opt)  
        {  
            case 'd':
                directoryPath = optarg;
                break;
            case 'c':
                cityNumbers = optarg;
                break;
            case 'r':
                IP = optarg;
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

    if(optind!=9){
        errno=EINVAL;
        printUsageExit(argv[0],"Missing argument"); 
    }

    if(PORT<0)
    {
        printUsageExit(argv[0], "Port number must be bigger than 0, preferable >2000");
    }

    struct sigaction newact;
    newact.sa_handler = &my_handler;
    newact.sa_flags = 0;

    if((sigemptyset(&newact.sa_mask) == -1) || (sigaction(SIGINT, &newact, NULL) == -1) ){
        termination();
        perror("Failled to install SIGINT signal handler");
        exit(EXIT_FAILURE);
    }

    country = calloc(1, sizeof(Country*));

    semap = sem_open(semapName,O_CREAT, 0644, 1); 
    if (semap == SEM_FAILED){
        perror("sem open");
        termination();
    }

    setPortNum();

    sprintf(bufWrite, "S%d %d %s ", portNum, (int) getPidFromProc(), getip());

    int start,end;
    sscanf( cityNumbers, "%d-%d", &start, &end);
    loadDataFromDisk(directoryPath, start,end,0);

    printf("Servant %d: loaded dataset, cities %s-%s\n",(int) getPidFromProc(), startCity, endCity);

    socketD = openSocket(socket1,PORT, IP);

    if(send(socketD, bufWrite, strlen(bufWrite), 0) < 0){
        perror("send");
        termination();
    }
    
    close(socketD);

    
    struct sockaddr_in serverAddr;
    int addrlen = sizeof(serverAddr);
    int opt1 = 1;

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
    serverAddr.sin_port = htons(portNum); 

    if(bind(socket1, (struct sockaddr *)&serverAddr, sizeof(serverAddr))<0) 
    {
        errExit("bind %s");
    }

    printf("Servant %d: listining at port %d\n", (int) getPidFromProc(), portNum);


    if (listen(socket1, BACKLOG) < 0) 
    {
        errExit("listen");
        exit(EXIT_FAILURE); 
    }

    int sNum=0,returnValue;

    if(signalArrived != 0){
        termination();
    }

    thread_id = (pthread_t*) calloc(500, sizeof(pthread_t));
    socketNums = (int*) calloc(500,sizeof(int));

    while(signalArrived == 0)
    {
        if(signalArrived == 0 && (socketD = accept(socket1, (struct sockaddr *)&serverAddr, (socklen_t*)&addrlen))<0)
        {
            if(signalArrived != 0)
            {
                termination();
            } 
        }

        if(sNum == 400){
            thread_id = (pthread_t*) realloc(thread_id, 700);
            socketNums = (int*) realloc(socketNums,700);
        }


        socketNums[sNum] = socketD;
        returnValue = pthread_create(&thread_id[sNum], NULL, threadFunc, &socketNums[sNum]);
        if (returnValue != 0){
            fprintf(stderr, "pthread_create supplier\n");
            exit(EXIT_FAILURE); 
        }
       
        sNum++;

    }
    close(socket1);

    exit(EXIT_SUCCESS); 

}


static void * threadFunc(void *arg)
{ 
    int socket = *(int*) arg;
    int totalTransactionNum = 0;
    bzero(bufWrite,sizeof(bufWrite));
    bzero(bufRead,sizeof(bufRead));

    read(socket, bufRead, sizeof(bufRead));

    char city[200], type[200], d1[200], d2[200];
    if(isalpha(bufRead[strlen(bufRead)-2]) == 0){ //no city
        sscanf(bufRead,"transactionCount %s %s %s\n",type, d1, d2);
        totalTransactionNum = searchInCountry(country,NULL,d1,d2, type);

    }
    else{
        sscanf(bufRead,"transactionCount %s %s %s %s\n",type, d1, d2,city);
        totalTransactionNum = searchInCountry(country,city,d1,d2, type);
    }
   
    char strTotalTransactionResult[30];
    sprintf(strTotalTransactionResult, "%d", totalTransactionNum);
    
      
    retValue = write(socket, strTotalTransactionResult, strlen(strTotalTransactionResult));
    if(retValue<0){
        perror("write");
        termination();
    }

    totalHandledRequest++;

    close(socket);
    return 0;
    
}

void printUsageExit(char* progName, char* message){
    fprintf(stderr, "%s\n",message );
    fprintf(stderr, "Usage: %s -d directoryPath -c 10-19 -r IP -p PORT\n", progName);
    exit(EXIT_FAILURE);
}

void loadDataFromDisk(char *path, int start, int end, int flag) {

    int cityNum = end-start;
    char cities[cityNum][40];

    if(flag == 0){
        struct dirent **namelist;
        int n;
        n = scandir(path, &namelist, NULL, alphasort);

        int i=0;
        if (n < 0){
            perror("scandir");
            exit(EXIT_FAILURE);
        }
        else {
            while (n--) {
                if (strcmp(namelist[n]->d_name, ".") != 0 && strcmp(namelist[n]->d_name, "..") != 0) {
                    if(n-1>=start && n-1<=end){
                        strcpy(cities[i], namelist[n]->d_name);
                        strcat(bufWrite," ");
                        strcat(bufWrite,cities[i]);
                        strcat(bufWrite," ");
                        if(i == 0)
                            strcpy(endCity, cities[i]);
                        else if(i == (end-start))
                            strcpy(startCity, cities[i]);

                        ++i;
                    }
                }
                free(namelist[n]);
            }
            free(namelist);
            flag=1;
        }
        
    }

    DIR *dir;
    struct dirent *dirent;

    dir = opendir(path);
    if (dir == NULL) {
        fprintf(stderr, "open dir error\n");
        exit(EXIT_FAILURE);
    }
    
    while ((dirent = readdir(dir)) != NULL) {
        
        if(dirent->d_type == DT_DIR) {
            for (int i = 0; i < cityNum+1; ++i)
            {
                if(strcmp(dirent->d_name,cities[i]) == 0){
                    newCity = cityInit(dirent->d_name);
                    char *currentPath = (char *)malloc(strlen(path) + strlen(dirent->d_name) + 3);
                    sprintf(currentPath, "%s/%s", path, dirent->d_name);
                    loadDataFromDisk(currentPath,start,end,flag);
                    insertCityToCountry(country,newCity);
                    free(currentPath);
                }
            }
        } 
        else {
            
            FILE *fp;
            char *newPath = (char *)malloc(strlen(path) + strlen(dirent->d_name) + 3);
            sprintf(newPath, "%s/%s", path, dirent->d_name);

            newDate = dateInit(dirent->d_name);

            char * line = NULL;
            size_t len = 0;
            ssize_t read;

            fp = fopen(newPath, "r");
            if (fp == NULL){
                perror("Open file");
                exit(EXIT_FAILURE);
            }
            while ((read = getline(&line, &len, fp)) != -1) {
                if(line[0] == ' '){
                    break;
                }
                newTransaction = transactionInit(line);
                insertTransactionToDate(newDate,newTransaction);

            }
            
            fclose(fp);

            insertDateToCity(newCity, newDate);

            if (line)
                free(line);
        }

    }

    closedir(dir);
}

pid_t getPidFromProc()
{
    char buf[32];
    int pid;
    
    readlink ("/proc/self", buf, sizeof (buf));
    sscanf (buf, "%d", &pid);
    return (pid_t) pid;
}


char* getip(){
    char hostbuffer[256];
    char *IPnumStr;
    struct hostent *host;
     int hostname;
  
    // To retrieve hostname
    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    if (hostname == -1)
    {
        perror("gethostbyname");
        exit(1);
    }
  
    host = gethostbyname(hostbuffer);
    if (host == NULL)
    {
        perror("gethostbyname");
        exit(1);
    }

    IPnumStr = inet_ntoa(*((struct in_addr*) host->h_addr_list[0]));


    return IPnumStr;
}

void setPortNum(){

    int bytesRead,bytesWritten;
    int portNumFD;
    int exist;

    if(sem_wait((semap)) == -1)
    {
        fprintf(stderr, "sem_wait\n");
        exit(EXIT_FAILURE);
    } 
    
    if (access(FILE_NAME, F_OK) == 0) {
        exist=1;
    } else {
        exist=0;
    }

    portNumFD = open (FILE_NAME, O_RDWR | O_CREAT);
    if(portNumFD<0){
        perror("Open file error");
        exit(EXIT_FAILURE); 
    }

    char buffer[6];
    bytesRead = read(portNumFD, buffer, 6);
    if(bytesRead<0){
        perror("Read error");
        exit(EXIT_FAILURE);
    }

    if(!exist){
        bytesWritten = write(portNumFD, "8000", strlen("8000"));
        if(bytesWritten<0){
            perror("Read error");
            exit(EXIT_FAILURE);
        }
        portNum = 8000;

    }
    else{
        if(lseek (portNumFD, 0, SEEK_SET)==-1){
            perror("Read error");
            exit(EXIT_FAILURE);
        }
        sscanf(buffer,"%d",&portNum);
        bzero(buffer,sizeof(buffer));
        portNum++;
        sprintf(buffer, "%d",portNum);
        bytesWritten = write(portNumFD, buffer, strlen("8000"));
        if(bytesWritten<0){
            perror("Read error");
            exit(EXIT_FAILURE);
        }
    }

    close(portNumFD);

    if(sem_post((semap)) == -1)
    {
        fprintf(stderr, "sem_post\n");
        exit(EXIT_FAILURE);
    }

}

void termination(){
    
    printf("Servant %d: termination message received, handled %d requests in total.\n",(int) getPidFromProc(),totalHandledRequest);

    if(socket1 != -1){
        close(socket1);
    }
    
    if(thread_id != NULL){
        free(thread_id);
    }
    free(country);

    remove(FILE_NAME);

    exit(EXIT_SUCCESS);
}