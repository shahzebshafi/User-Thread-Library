# User-Level Thread Library

## Summary

The user thread library that we built can create different threads within one process using a queue and the threads are
scheduled in a round-robin fashion so that each thread runs a little bit at a time. The threads are also preemptive 
meaning there's an internal timer to interrupts the threads. Also the threads are synchornized using semaphores so that
there's no overlapping of changing/accessing variables when a thread is running.

## Implementation

The implementation for this program contained five steps

1. Creating a queue to hold the threads
2. Building the thread library which creates and runs the threads
3. Creating semaphores so that each thread is synchornized with each other and not overlapping
4. Build a timer-based interrupt system which interrupts the threads while they're running
5. Testing each individual part to make sure it works correctly


### Creating Queue

Built a simple queue that contains the functions to create a queue, destroy a queue, enqueue an
item to the back of the queue, dequeue an item from the front queue, and delete an item from
the queue. This queue also contains a function that iterates through the entire queue applying
a given function to each element. To do this two structures were created, one to hold the entire
queue and the other to hold an individual queue node. The queue structure contains the head and
tail of the queue, and a counter to hold the number of elements. The queue node structure 
contains a pointer to the next node and the data in the queue node. When the queue is created, 
memory is allocated, the head and tail are set to NULL, and the counter is set to 0 and the
queue is then returned. The destory function frees the queue's memory that was allocated in
create. queue_enqueue() creates a new node, allocates memory for it, puts the data that was
passed in from the function into the new_node's data pointer, and the next node is set to NULL.
Depending on if the new node is the head or not the queue->tail gets updated accordingly.
queue_dequeue() sets the data pointer which was passed in equal to the head which is getting
dequeud and head element gets changed, and the old head gets it's memory deallocated. In
queue_delete() entire queue gets iterated through until the data that will be deleted is found
or not found. If the data is found then it's checked whether the node is the head or not and it
gets updated accordingly, then the deleted node is deallocated. queue_iterate() goes through
each element in the queue and applies the function to each element using the function that was
passed in. queue_length() returns the number of queue nodes because the queue is incremented or
decremented with each enqueue or dequeue/delete.

### Building the thread library

Our thread library contains the thread control block as a structure and it contains functions to create, run, unblock, block, 
exit from, and yield threads. uthread_yield() works by saving the current thread into another tcb called old_thread, then the
next thread in the queue is dequeued and is made the active thread and set to the running state. It then checks to see if the 
old_thread is either running or ready and if it is then it gets set to a ready state and is enqueued to the back of the queue.
At the end of this function uthread_ctx_switch() is used to start running the new thread and stop the old one. The
uthread_create() function creates a new tcb for the new thread and puts all the necessary information into the structure
such as it's state (which is set to ready), its data, and its function, and then it's enqueued to the back of the queue.
uthread_run() first checks for preemption and if preemption is true then the interrupt timer is created, then it allocates
memory for this main thread that's going to run and it's context. If this is the first thread, then it creates an initial
thread and creates two queues, it then sets the main thread to idle and it's arg and function to NULL because at this
point it's not running, then the thread that was passed in is sent to be created in the uthread_create() function. Then the
intial thread state is set to running and the intial thread is put in the queue, while the main function is put in the 
idle_queue since it's not executing/running anything.Then there's a loop to go through all the threads in the loop. If there
is only one thread in the queue and it is dequeued so that it's context can be switched with the main_thread, if there is 
more than one thread, the yield function gets called and the loop continues. uthread_block() first disables preemption because 
this is a critical section. A new tcb is created so that the blocked thread can be stored and that thread's state is changed
to blocked, then another tcb is created to hold the new thread which is put in to the queue and set to running, then
uthread_ctx_switch is used to switch from the old thread's context to the new context, and at the end preemption get enabled
again. uthread_unblock starts with disabling preemption and setting the state of the thread that asked to be unblocked to
running and the active thread is set to idle, and the thread that asked to be unblocked context switches with the active
thread, then preemption is enabled. Our thread library has another function which we added called MoveToTop() which moves
any thread we're working with to the top of the queue, the reason we did this is because we were having trouble tracking
the order of the queue, so we created this function to move any node to the top of the queue in case it needs to be dequeued
with the data returning to the user. And this function is called through the library to move certain nodes to the top of the
queue



### Creating Semaphores

Our semamphore contains a counter and a queue to hold the threads waiting for a resource. sem_create() creates the semaphore
and creates the wait_queue and creates the semaphore to be of the size specified. sem_destroy() first destroys the queue using
queue_destroy and then it frees the memory that was allocated for the semaphore. sem_down() decrements count if it is one or more.
If the count is equal to 0 and there are no resources available then a new thread is created and it dequues the current thread, then
enqueues the thread to the semaphore's wait queue which means it's now in line for it's resource, then uthread_block() is called
to block the current thread from executing. When sem_up() is called the counter gets incremented by one and a new thread is created
and allocated, then it's checked if there any threads ahead in waiting queue, and if they are then a thread is dequeued from the 
wait queue, and if there are resources available then the dequeued thread gets unblocked meaning it can keep running, and if
there are no resources then the thread is enqueued at the back of the queue.


### Building preemptive timer

The preempt interrupt timer is built using sigaction and itimerval which were referenced from
the GNU C library. There are three global variables in this section which are the old action,
the old timer and the block signal. When preemption is started then the signals that are in the
block signal are emptied so it contains no signals, then the SIGVTALARM is added to the block
signal which is all it contains. The SIGVTALARM is what's used to block/preempt the current
thread. Then the new action gets set up and a handler gets created which is used when a
SIGVTALARM is recieved, and the action signal set is then emptied so none of the signals are 
blocked when the signal handler is running and the flags are removed. When this is set up,
sigaction() is used to to change the action when SIGVTALARM is received and the old action gets 
saved, the reason the old action gets saved even at the start is because an interrupt timer
should always be resuming, never starting. Then the timer is created using the itimerval
structure which is used to specify when an interrupt with the SIGVTALARM signal is sent. 
it_value() is used to set when the first interrupt which will be at 0.01 seconds and the
it_interval() is used to set the interval between each timer which is also every 0.01 seconds.
Then the setitimer() is used to start a virtual timer (ITIMER_VIRTUAL) which contains the timer 
structure that was created earlier and set up to send interrupts every 0.01 seconds, and the
old timer is also saved for the same reason that the old action was saved. In the preempt_stop
() function the both the old action and the old timer are restored using sigaction() and
setitimer(). preempt_disable() blocks the SIGVTALARM from being recieved by putting SIGBLOCK into the
block signal set using sigprocmask(), there are no more interrupts after this is called. preempt_enable() 
unblocks the SIGVTALARM from being recieved, so the timer interrupts start again at 0.01 seconds.

### Testing

The testing for the thread library and the semaphore was done using the provided testers such as 
uthread_hello, uthread_yield. For the queue the provided tests which included creating the queue, enqueing/dequeueing, 
and applying a function to all the queue nodes. For all the queue functions we tested to make sure they returned a -1 (error) when
either a null queue or null data was sent. Also we tested the queue_delete by having it delete an item in the middle of the 
queue, and also tested it by making sure it returned an error when queue_delete() can't find the data it'slooking to delete. 
We used a simple test for testing preemption where we print a message in each thread a million times
and those messages should be interweaved with one another if the preemption actually works as intended. The only testing we had trouble with was the semaphore tests, the sem_simple.c test passed but the other semaphore tests did not.