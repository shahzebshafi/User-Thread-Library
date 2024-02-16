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

/*Global Variabales that hold the old signal action, the old timer, and the block signal*/
static struct sigaction old_action;
static struct itimerval old_timer;
static sigset_t block_sig;

void signal_handler(int signal)
{
    /*Call uthread_yield when SIGVTALRM is received*/
    if (signal == SIGVTALRM) {
            uthread_yield();
    }
}

void preempt_disable(void)
{
    /*Error checking to make sure the block signal is blocked*/
   if(sigprocmask(SIG_BLOCK, &block_sig, NULL) == -1){ 
       perror("sigprocmask");
       exit(1);
   }
}

void preempt_enable(void)
{
    /*Error checking to make sure the block signal is not blocked*/
   if(sigprocmask(SIG_UNBLOCK, &block_sig, NULL) == -1){ 
       perror("sigprocmask");
       exit(1);
   }

}

void preempt_start(bool preempt)
{
    if(preempt){
        /*Adds the SIGVTALARM to the block signal set which is the only signal it contains*/
        sigemptyset(&block_sig);
        sigaddset(&block_sig, SIGVTALRM);

        struct sigaction action; /*Referenced from GNU C library section 24.3.4*/

        /*Set up the structure for the new action*/
        action.sa_handler = signal_handler;
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;

        /*Error checking to make sure new action is taken when SIGVTALARM is recieved, and the old action is saved*/
        if(sigaction(SIGVTALRM, &action, &old_action) == -1){
            perror("sigaction");
            exit(1);
        }

        struct itimerval timer;  /*Referenced from GNU C library section 21.6*/

        /*Set up the structure for the timer that generates the SIGVTALARM, and set the interrupts to 0.01 seconds*/
        timer.it_interval.tv_sec = 0;
        timer.it_interval.tv_usec = 1000000 / HZ;
        timer.it_value.tv_sec = 0;
        timer.it_value.tv_usec = 1000000 / HZ;

        /*Error checking to make sure the new timer is set, and the old timer is saved*/
        if(setitimer(ITIMER_VIRTUAL, &timer, &old_timer) == -1){
            perror("setitimer");
            exit(1);
        }

    }
}

void preempt_stop(void)
{
    /*Error checking to make sure the old action is restored properly */
    if (sigaction(SIGVTALRM, &old_action, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    /*Error checking to make sure the old timer is restored properly */
    if (setitimer(ITIMER_VIRTUAL, &old_timer, NULL) == -1) {
        perror("setitimer");
        exit(1);
    }
}

