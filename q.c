
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
 

struct Queue {                      // A structure for a q
    int front, rear, size;
    unsigned cap;
    int* array;
};
 
// function to create a queue

struct Queue* createQueue(unsigned cap)
{
    struct Queue* queue = (struct Queue*)malloc(sizeof(struct Queue));
    
    queue->cap = cap;
    queue->front = 0;
    queue->size = 0;
    queue->rear = cap - 1;
    queue->array = (int*)malloc(queue->cap * sizeof(int));
    
    return queue;
}
 
 
int isFull(struct Queue* queue)             // function checks if q is full
{
    return (queue->size == queue->cap);
}
 

int isEmpty(struct Queue* queue)          // function checks if q is emtpy
{
    return (queue->size == 0);
}
 


void enqueue(struct Queue* queue, int item)     // Function to add an item to the queue.
{
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1) % queue->cap;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
    
    printf("%d enqueued to q\n", item);
}
 


int dequeue(struct Queue* queue)            // Function to remove an item from queue.
{
    int item;
    
    if (isEmpty(queue))
        return INT_MIN;
    item = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->cap;
    queue->size = queue->size - 1;
    
    return item;
}
 
int front(struct Queue* queue)      // Function gets value at the front
{
    if (isEmpty(queue))
        return INT_MIN;
    
    return queue->array[queue->front];
}
 

int rear(struct Queue* queue)      // function gets value at the rear
{
    if (isEmpty(queue))
        return INT_MIN;
    
    return queue->array[queue->rear];
}
 

int main()
{
 
    return 0;
}
