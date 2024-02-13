#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

struct queue_node {
        void *data;
        struct queue_node *next;
};

struct queue {
        int count;
        struct queue_node *head;
        struct queue_node *tail;        
};

queue_t queue_create(void)
{
        queue_t queue = malloc(sizeof(struct queue));
        if (queue == NULL) {
                return NULL;
        }
        queue->count = 0;
        queue->head = NULL;
        queue->tail = NULL;
        return queue;
}

int queue_destroy(queue_t queue)
{
        if(queue == NULL || queue->count != 0) {
                return -1;
        }
        free(queue);
        return 0;
}

int queue_enqueue(queue_t queue, void *data)
{
        if (queue == NULL || data == NULL) {
                return -1;
        }
        struct queue_node *new_node = malloc(sizeof(struct queue_node));
        if (new_node == NULL) {
                return -1;
        }
        new_node->data = data;
        new_node->next = NULL;
        if (queue->count == 0) {
                queue->head = new_node;
                queue->tail = new_node;
        } 
        else {
                queue->tail->next = new_node;
                queue->tail = new_node;
        }
        queue->count++;
        return 0;
}

int queue_dequeue(queue_t queue, void **data)
{
        if (queue == NULL || data == NULL || queue->count == 0) {
                return -1;
        }
        *data = queue->head->data;
        struct queue_node *temp = queue->head;
        queue->head = queue->head->next;
        free(temp);
        queue->count--;
        return 0;
}

int queue_delete(queue_t queue, void *data)
{
      if(queue == NULL || data == NULL || queue->count == 0) {
                return -1;
        }
        struct queue_node *temp = queue->head;
        struct queue_node *prev = NULL;
        while(temp != NULL) {
                if(temp->data == data) {
                        if(prev == NULL) {
                                queue->head = temp->next;
                        } 
                        else 
                        {
                                prev->next = temp->next;
                        }
                        free(temp);
                        queue->count--;
                        return 0;
                }
                prev = temp;
                temp = temp->next;
        }
        return -1;
}

int queue_iterate(queue_t queue, queue_func_t func)
{
        if(queue == NULL || func == NULL) {
                return -1;
        }
        struct queue_node *temp = queue->head;
        while(temp != NULL) {
                struct queue_node *next = temp->next;
                func(queue, temp->data);
                temp = next;
        }
        return 0;
}

int queue_length(queue_t queue)
{
        if(queue == NULL) {
                return -1;
        }
        return queue->count;
}
