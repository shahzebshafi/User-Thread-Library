#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"

#define HZ 100

static struct sigaction old_action;
static struct itimerval old_timer;
static sigset_t block_sig;

void signal_handler(int signal)
{
    if (signal == SIGVTALRM) {
            uthread_yield()
    }
}

void preempt_disable(void)
{
   if(sigprocmask(SIG_BLOCK, &block_sig, NULL) == -1){ /*Blocks the SIGVTALARM*/
       perror("sigprocmask");
       exit(1);
   }
}

void preempt_enable(void)
{
   if(sigprocmask(SIG_UNBLOCK, &block_sig, NULL) == -1){ /*Unblocks the SIGVTALARM*/
       perror("sigprocmask");
       exit(1);
   }

}

void preempt_start(bool preempt)
{
    if(preempt){
        struct sigaction action; /*GNU C library*/
        action.sa_handler = signal_handler;
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;

        if(sigaction(SIGVTALRM, &action, &old_action) == -1){
            perror("sigaction");
            exit(1);
        }
        sigemptyset(&block_sig);
        sigaddset(&block_sig, SIGVTALRM);

        struct itimerval timer;  /*GNU C library*/
        timer.it_interval.tv_sec = 0;
        timer.it_interval.tv_usec = 1000000 / HZ;
        timer.it_value.tv_sec = 0;
        timer.it_value.tv_usec = 1000000 / HZ;

        printf("Preempt_start\n");
        if(setitimer(ITIMER_VIRTUAL, &timer, &old_timer) == -1){
            perror("setitimer");
            exit(1);
        }

    }
}

void preempt_stop(void)
{
    // Restore the old signal handler
    if (sigaction(SIGVTALRM, &old_action, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    // Restore the old timer
    if (setitimer(ITIMER_VIRTUAL, &old_timer, NULL) == -1) {
        perror("setitimer");
        exit(1);
    }
    printf("preempt_stop\n");
}
