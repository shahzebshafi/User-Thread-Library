#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <uthread.h>

void thread2(void *arg){
    (void)arg;


    for(int i = 0; i < 1000000; i++)
        printf("--------------------------------------------\n");

}

void thread1(void *arg){
    (void)arg;

    uthread_create(thread2, NULL);

    for(int i = 0; i < 1000000; i++)
        printf("+++++++++++++++++++++++++++++++++++++++++++++++++\n");

}

int main(void) {
    /*Preemption is enabled here, so the print message for thread 1 and 2 should be interweaved, instead of running sequentially*/
    uthread_run(true, thread1, NULL);

    return 0;
}