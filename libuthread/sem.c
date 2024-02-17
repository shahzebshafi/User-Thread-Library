#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "queue.h"
#include "sem.h"
#include "private.h"
#include "uthread.h"

extern queue_t queue;
/* sem struct that holds the number of available resources and a queue with threads waiting for sem */
typedef struct semaphore {
	size_t count;
	queue_t wait_queue;
}semaphore;

/* uthread control block that holds the state of the threads, context, and the top of the stack */
typedef struct uthread_tcb{
	int state;
	uthread_ctx_t *context;
	uthread_func_t func;
	void *arg;
	void *top_of_stack;
}uthread_tcb;

sem_t sem_create(size_t count)
{
	semaphore* sem = malloc(sizeof(semaphore));
	if(!sem){
		return NULL;
	}
	/* create queue for sem and set the initial count */
	sem->wait_queue = queue_create();
	sem->count = count;
	return sem;
}

int sem_destroy(sem_t sem)
{
	/* if sem is NULL or if threads are waiting */
	if(!sem || queue_length(sem->wait_queue) > 0){
		return -1;
	}
	/* free memory allocated to sem and destroy the queue */
	queue_destroy(sem->wait_queue);
	free(sem);
	return 0;	
}

int sem_down(sem_t sem)
{
	/* if sem is NULL or if there is no wait queue return error */
	if(!sem || !sem->wait_queue){
		return -1;
	}
	/* decrement resources if sem count is 1 or more*/
	if(sem->count >= 1){
		sem->count -= 1;
		return 0;
	}
	/* if there are no more resources available */
	if(sem->count == 0){
		uthread_tcb *thread;
		thread = malloc(sizeof(uthread_tcb));
		/* get current thread and queue new thread and block thread */
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
	/* increment count of resources */
	sem->count ++;
	uthread_tcb* thread;
	thread = malloc(sizeof(uthread_tcb));
	/* if sem queue is empty or sem is NULL return -1 */
	if(!sem->wait_queue || !sem){
		return -1;
	}
	/* if threads are in the queue and resources are available then unblock */
	if(queue_length(sem->wait_queue) > 0){
		if(!queue_dequeue(sem->wait_queue,(void**)&thread)){
			if(sem->count > 0){
				uthread_unblock(thread);
			}
			/* if resources are not available then add to wait queue */
			else{
				queue_enqueue(sem->wait_queue,(void*)thread);
			}
		}
	}
	return 0;
}
