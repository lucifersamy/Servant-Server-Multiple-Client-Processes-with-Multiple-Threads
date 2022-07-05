#ifndef QUEUE
#define QUEUE

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*Keeping request sockets*/
struct Node{
    int requestSocketFd;
    struct Node *next;
};

/*Keeping nodes in queue*/
struct Queue{
    struct Node *front;
    struct Node *rear;
};

typedef struct Queue Queue;
typedef struct Node Node;

/*Check if the queue is empty*/
int isEmpty(Queue *queue);
/*enequeue new socket*/
void enqueue(Queue *queue, int newSocket);
/*dequeue new socket*/
int dequeue(Queue *queue);



#endif