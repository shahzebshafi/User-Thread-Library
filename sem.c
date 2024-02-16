#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "queue.h"
#include "sem.h"
#include "private.h"
#include "uthread.h"

typedef struct semaphore {
	size_t count;
	queue_t wait_queue;
}semaphore;

typedef struct uthread_tcb{
	int state;
	uthread_ctx_t *context;
	uthread_func_t func;
	void *arg;
	void *top_of_stack;
}uthread_tcb;

static queue_t queue;

sem_t sem_create(size_t count)
{
	semaphore* sem = malloc(sizeof(semaphore));
	if(!sem){
		return NULL;
	}
	sem->wait_queue = queue_create();
	sem->count = count;
	return sem;
}

int sem_destroy(sem_t sem)
{
	if(!sem || queue_length(sem->wait_queue) > 0){
		return -1;
	}
	queue_destroy(sem->wait_queue);
	free(sem);
	return 0;	
}

int sem_down(sem_t sem)
{
	if(!sem || !sem->wait_queue){
		return -1;
	}
	if(sem->count >= 1){
		sem->count -= 1;
		return 0;
	}
	if(sem->count == 0){
		uthread_tcb *thread;
		thread = malloc(sizeof(uthread_tcb));
		thread = uthread_current();
		if(!queue_dequeue(queue,(void**)&thread)){
			queue_enqueue(sem->wait_queue,(void*)thread);
			uthread_block();
		}
	}
	return 0;
}

int sem_up(sem_t sem)
{
	sem->count ++;
	uthread_tcb* thread;
	thread = malloc(sizeof(uthread_tcb));
	if(!sem->wait_queue || !sem){
		return -1;
	}
	if(queue_length(sem->wait_queue) > 0){
		if(!queue_dequeue(sem->wait_queue,(void**)&thread)){
			if(sem->count > 0){
				uthread_unblock(thread);
			}
			else{
				queue_enqueue(sem->wait_queue,(void*)thread);
			}
		}
	}
	return 0;
}

