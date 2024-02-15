#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "queue.h"
#include "sem.h"
#include "private.h"

struct semaphore {
	size_t count;
	queue_t wait_list;
};
	
extern queue_t queue;

struct uthread_tcb{
	int state;
	uthread_ctx_t *context;
	uthread_func_t func;
	void *arg;
	void *top_of_stack;
};

sem_t sem_create(size_t count)
{
	struct semaphore* new_sem = malloc(sizeof(struct semaphore));
	new_sem->wait_list = queue_create();
	if(new_sem == NULL){
		return NULL;
	}
	else{
		new_sem->count = count;
		return new_sem;
	}
}

int sem_destroy(sem_t sem)
{
	if(sem ==NULL || queue_length(sem->wait_list) > 0){
		return -1;
	}
	free(sem);
	return 0;	
}

int sem_down(sem_t sem)
{
	if(sem->count >= 1){
		--sem->count;
		return 0;
	}
	else{
		struct uthread_tcb* new_thread;
		new_thread = malloc(sizeof(struct uthread_tcb));
		new_thread = uthread_current();
		if(!queue_dequeue(queue,(void**)&new_thread)){
			queue_enqueue(sem->wait_list,(void*)new_thread);
			uthread_block();
		}
	}
	return -1;
}

int sem_up(sem_t sem)
{
	sem->count++;
	struct uthread_tcb* new_thread;
	new_thread = malloc(sizeof(struct uthread_tcb));
	if(queue_length(sem->wait_list) > 0){
		if(!queue_dequeue(sem->wait_list,(void**)&new_thread)){
			if(sem->count > 0){
				uthread_unblock(new_thread);
			}
			else{
				queue_enqueue(sem->wait_list,(void*)new_thread);
			}
		}
	}
	return 0;
}

