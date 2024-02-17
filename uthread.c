#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"
#include "queue.h"

/* Thread States */
#define blocked 0
#define ready 1
#define running 2
#define idle 3
#define exited 4


typedef struct uthread_tcb {
	int state;
	uthread_ctx_t *context;
	uthread_func_t func;
	void *arg;
	void *top_of_stack;
}uthread_tcb;

/* thread queue */
queue_t queue;
queue_t idle_q;
/* control block threads */
uthread_tcb *main_thread;
uthread_tcb *active_thread;

struct uthread_tcb *uthread_current(void)
{
	return active_thread;
}

/* running thread will be moved to top of queue */
void MoveToTop(queue_t queue){
	
	/* create a queue for for the threads */
	queue_t wait = queue_create();

	/* active thread */
	active_thread = malloc(sizeof(uthread_tcb));
	active_thread->top_of_stack = uthread_ctx_alloc_stack();
	active_thread->context = uthread_ctx_alloc_stack();

	/* create a current thread */
	uthread_tcb *uthread_current;
	uthread_current = malloc(sizeof(uthread_tcb));
	uthread_current->top_of_stack = uthread_ctx_alloc_stack();
	uthread_current->context = uthread_ctx_alloc_stack();

	for(int i = 0, q_length = queue_length(queue); i < q_length; i++){
		if(!queue_dequeue(queue,(void**)&uthread_current)){
			if(uthread_current->state != running){
				queue_enqueue(wait,(void*)uthread_current);
			}
			else{
				active_thread = uthread_current;
			}
		}
	}
	/* add the active thread to the top of queue */
	queue_enqueue(queue, active_thread);

	for(int i = 0, wait_length = queue_length(wait); i < wait_length; i++){
		if(!queue_dequeue(wait,(void**)&uthread_current)){
			queue_enqueue(queue,(void*)uthread_current);
		}
	}
}


/* Referenced: https://man7.org/linux/man-pages/man3/pthread_yield.3.html, https://www.geeksforgeeks.org/java-concurrency-yield-sleep-and-join-methods/ */
void uthread_yield(void)
{
    if (queue_length(queue) > 0) {
        uthread_tcb *old_thread = active_thread;
        if (queue_dequeue(queue, (void**)&active_thread) == 0) {
            active_thread->state = running;
			/* add old_thread to queueu if it is running or ready */
            if (old_thread->state == running || old_thread->state == ready) {
				old_thread->state = ready;
                queue_enqueue(queue, old_thread);
            }
			/* switch to the new thread */
            uthread_ctx_switch(old_thread->context, active_thread->context);
        }
    }
}


void uthread_exit(void)
{
    active_thread->state = exited;
    queue_delete(queue, active_thread);

    if (queue_length(queue) > 0) {
        uthread_yield();
    }
	/* if there are no more threads in queue then switch to idle threads */
    else {
        uthread_tcb *next_thread;
        if (!queue_dequeue(idle_q, (void**)&next_thread)) {
            uthread_ctx_switch(active_thread->context, next_thread->context);
        }
    }
}

int uthread_create(uthread_func_t func, void *arg)
{
	/* create new thread */
	uthread_tcb *new_thread = malloc(sizeof(uthread_tcb));
	if(new_thread == NULL){
		return -1;
	}
	/* set all variables for the new thread */
	new_thread->context = malloc(sizeof(uthread_ctx_t));
	new_thread->func = func;
	new_thread->arg = arg;
	new_thread->state = ready;
	new_thread->top_of_stack = uthread_ctx_alloc_stack();
	/* if the new thread is Null or uthread_ctx_init is -1 return error */
	if(new_thread->top_of_stack == NULL || uthread_ctx_init(new_thread->context, new_thread->top_of_stack, 
	new_thread->func, new_thread->arg) == -1){
		return -1;
	}
	/* add the new thread to the queue */
	queue_enqueue(queue,new_thread);

	return 0;
}

int uthread_run(bool preempt, uthread_func_t func, void *arg)
{
	/* enable preempt if called */
	if(preempt){
		preempt_start(preempt);
		preempt_enable();
	}
	/* create main thread */
	uthread_tcb *main_thread = malloc(sizeof(uthread_tcb));
	main_thread->context = malloc(sizeof(uthread_ctx_t));
	if (main_thread == NULL||main_thread->context == NULL){
		return -1;
	}
	/* if queue is empty then create new thread and make main thread idle */
	if(queue == NULL){
		uthread_tcb *initial_thread;
		initial_thread = malloc(sizeof(uthread_tcb));
		initial_thread->top_of_stack = uthread_ctx_alloc_stack();
		initial_thread->context = uthread_ctx_alloc_stack();
		/* create new queue */
		queue = queue_create();
		idle_q = queue_create();
		main_thread->func = NULL;
		main_thread->state = idle;
		main_thread->arg = NULL;
		uthread_create(func, arg);
		/* if initial thread is not dequeued then initial thread is running and enqueued */
		if(!queue_dequeue(queue,(void**)&initial_thread)) {
			initial_thread->state = running;
			queue_enqueue(queue,(void*)initial_thread);
			queue_enqueue(idle_q,(void*)main_thread);
			MoveToTop(queue);
			/* switch threads */
			uthread_ctx_switch(main_thread->context,initial_thread->context);
		}
	}
	while(queue_length(queue) >= 1){
		/* if it is the last thread */
		uthread_tcb *last_thread;
		last_thread = malloc(sizeof(uthread_tcb));
		last_thread->top_of_stack = uthread_ctx_alloc_stack();
		last_thread->context = uthread_ctx_alloc_stack();
		if(queue_length(queue) == 1){
			if(!queue_dequeue(queue,(void**)&last_thread)){
				last_thread->state = running;
				uthread_ctx_switch(main_thread->context,last_thread->context);
			}
			return 0;
		}
		else{
			uthread_yield();
		}
	}

	return 1;
}

void uthread_block(void)
{
	preempt_disable();
	/* create a blocked thread and next thread */
	uthread_tcb* blocked_thread;
	blocked_thread = malloc(sizeof(uthread_tcb));
	blocked_thread = active_thread;
	blocked_thread->state = blocked;

	uthread_tcb* next_thread;
	next_thread = malloc(sizeof(uthread_tcb));

	/* if next_thread is not dequeued then it is running */
	if(!queue_dequeue(queue,(void**)&next_thread)){
		queue_enqueue(queue,next_thread);
		next_thread->state = running;
		MoveToTop(queue);
		uthread_ctx_switch(blocked_thread->context,next_thread->context);
	}
	else {
		/* check if next thread was dequeued from idle queue */
		if (!queue_dequeue(idle_q,(void**)&next_thread)){
			uthread_ctx_switch(blocked_thread->context,next_thread->context);
		}
	}
	preempt_enable();
}

void uthread_unblock(uthread_tcb *uthread)
{
	preempt_disable();
	MoveToTop(queue);
	/*if active thread is not dequeued then active thread is idle and uthread is running */
	if(!queue_dequeue(queue,(void**)&active_thread)){
		uthread->state = running;
		active_thread->state = idle;
		uthread_ctx_switch(active_thread->context,uthread->context);
	}
	preempt_enable();
}