#include "Queue.h"

int isEmpty(Queue *queue){

    if(queue->front==NULL){
        return 0;
    }
    
    return 1;
}

void enqueue(Queue *queue, int newSocket){

    Node* temp = (Node *) malloc(sizeof(Node));
    temp->next = NULL;
    temp->requestSocketFd = newSocket;

    if(queue->front != NULL)
        queue->rear->next = temp;
    else
        queue->front = temp;
    
    queue->rear = temp;
}



int dequeue(Queue *queue)
{     
    Node *temp;                                 
    int socket;

    if(queue->front != NULL){
        socket = queue->front->requestSocketFd;
        temp = queue->front;
        queue->front = queue->front->next;
        
        free(temp);
        
        if(queue->front==NULL)
            queue->rear=NULL;
    }

    return socket;
}