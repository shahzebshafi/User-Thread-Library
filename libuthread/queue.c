#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/*Individual queue node*/
struct queue_node {
        void *data;
        struct queue_node *next;
};

/*Entire Queue*/
struct queue {
        int count;
        struct queue_node *head;
        struct queue_node *tail;        
};

/*Create a new queue*/
queue_t queue_create(void)
{
        queue_t queue = malloc(sizeof(struct queue));
        /*Error checking to see if queue was allocated correctly */
        if (queue == NULL) {
                return NULL;
        }
        queue->count = 0;
        queue->head = NULL;
        queue->tail = NULL;
        /*Return pointer to new queue*/
        return queue;
}

int queue_destroy(queue_t queue)
{
        /*Error checking to make sure queue is not NULL or not empty*/
        if(queue == NULL || queue->count != 0) {
                return -1;
        }
        /*Deallocate the memory that the queue was using*/
        free(queue);
        return 0;
}

int queue_enqueue(queue_t queue, void *data)
{
        /*Error checking to make sure queue and data are not NULL*/
        if (queue == NULL || data == NULL) {
                return -1;
        }
        /*Create the new node that will be enqued*/
        struct queue_node *new_node = malloc(sizeof(struct queue_node));
        /*Memory allocation error checking*/
        if (new_node == NULL) {
                return -1;
        }
        /*Put the data that was passed into the queue and set the next node equal to NULL*/
        new_node->data = data;
        new_node->next = NULL;
        /*If it's the first element then there's only one element in the queue, so both the head and the tail are equal to that one node */
        if (queue->count == 0) {
                queue->head = new_node;
                queue->tail = new_node;
        }
        /*If it's not the first element then it's added to the back of the queue, and the tail pointer get updated */
        else {
                queue->tail->next = new_node;
                queue->tail = new_node;
        }
        /*Count is incremented because there's one more element*/
        queue->count++;
        return 0;
}

int queue_dequeue(queue_t queue, void **data)
{
        /*Error checking to make sure both queue and data are not NULL and that the queue isn't empty*/
        if (queue == NULL || data == NULL || queue->count == 0) {
                return -1;
        }
        /*Set the data pointer equal to the data head node which is what is being dequeued, and create a temp pointer to hold the address*/
        *data = queue->head->data;
        struct queue_node *temp = queue->head;
        /*Set the head to the next element in the queue*/
        queue->head = queue->head->next;
        /*The old head is released and the queue gets decremented*/
        free(temp);
        queue->count--;
        return 0;
}


int queue_delete(queue_t queue, void *data)
{
        /*Error checking to make sure queue and data are not NULL*/
        if(queue == NULL || data == NULL) {
                return -1;
        }
        /*Create a temporary pointer that points to the head, and a pointer that follows behind*/
        struct queue_node *temp = queue->head;
        struct queue_node *prev = NULL;
        /*Iterate through the entire queue to find the node which needs to be deleted*/
        while(temp != NULL) {
                /*If the data is found, then check which element in the queue it is*/
                if(temp->data == data) {
                        /*If it's the head of the queue, then set the new head to the second element in the queue*/
                        if(prev == NULL) {
                                queue->head = temp->next;
                        } 
                        /*If it's not the head, then set the previous node's next node to the node that's being deleted next node, so the queue is still connected after the delete*/
                        else {
                                prev->next = temp->next;
                        }
                        /*Deallocate the deleted node and decrement the queue, and return 0 when it's deleted*/
                        free(temp);
                        queue->count--;
                        return 0;
                }
                /*If the data isn't in the current node, go to the next node*/
                prev = temp;
                temp = temp->next;
        }
        /*Error checking to see if data is not found in the queue*/
        return -1;
}

int queue_iterate(queue_t queue, queue_func_t func)
{
        /*Error checking to make sure data and func are not NULL*/
        if(queue == NULL || func == NULL) {
                return -1;
        }
        /*Create a temporary pointer that points to the head*/
        struct queue_node *temp = queue->head;
        /*Iterate through all the elements*/
        while(temp != NULL) {
                /*Call the function on each node in the queue*/
                struct queue_node *next = temp->next;
                func(queue, temp->data);
                temp = next;
        }
        return 0;
}

int queue_length(queue_t queue)
{
        /*Error checking to make sure queue is not NULL*/
        if(queue == NULL) {
                return -1;
        }
        /*Return number of nodes in the queue*/
        return queue->count;
}
