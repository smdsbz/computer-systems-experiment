/*
 * Author: Xiaoguang Zhu <github.com/smdsbz>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <signal.h>
typedef void (*sighandler_t)(int);


/* Process Runtime Environment */

pid_t pid_pool[2] = { 0 };

typedef int pipefd_pair_t[2];
pipefd_pair_t pipefds = { 0 };  // read / write, respectively


/* Signal Handlers */

// void do_nothing_handler(int signo) { return; }


/*interrupt*/ void main_signal_handler(int signo) {
    switch (signo) {
        case SIGINT: {  // <Ctrl-C>
            // kill all child processes
            for (unsigned idx = 0; idx != 2; ++idx) {
                kill(pid_pool[idx], SIGUSR1);
            }
            // wait for all childs to terminate
            while (wait(NULL) > 0) ;
            /* perror("wait(NULL)");   // "No child processes" */
            printf("[main] Parent Process is Killed!\n");
            exit(EXIT_SUCCESS);
        }
        default: { break; }
    }
    return;
}


/*interrupt*/ void child_1_signal_handler(int signo) {
    switch (signo) {
        case SIGUSR1: {
            // close writing end of pipe
            close(pipefds[1]);
            printf("[child 1] Child Process 1 is Killed by Parent!\n");
            exit(EXIT_SUCCESS);
        }
        default: { break; }
    }
    return;
}


/*interrupt*/ void child_2_signal_handler(int signo) {
    switch (signo) {
        case SIGUSR1: {
            // close reading end of pipe
            close(pipefds[0]);
            printf("[child 2] Child Process 2 is Killed by Parent!\n");
            exit(EXIT_SUCCESS);
        }
        default: { break; }
    }
    return;
}


/* Child Processes */

/*
 * Child Process 1 - Writer of pipe comm.
 */
void child_1_fn(void) {
    // register signals
    signal(SIGUSR1, child_1_signal_handler);
    signal(SIGINT, SIG_IGN);
    // close unused reading end of pipe
    close(pipefds[0]);
    // send message per second
    char buf[100] = { 0 };
    unsigned cnt = 1;
    while (1) {
        snprintf(buf, 100, "I send you %d times.\n", cnt++);
        write(pipefds[1], buf, strlen(buf));
        sleep(1);
    }
    // NOTE: process ends on SIGUSR1
    exit(EXIT_FAILURE);
}


/*
 * Child Process 2 - Reader of pipe comm.
 */
void child_2_fn(void) {
    // register signals
    signal(SIGUSR1, child_2_signal_handler);
    signal(SIGINT, SIG_IGN);
    // close unused writing end of pipe
    close(pipefds[1]);
    // read message from pipe and print to console
    char buf = '\0';
    while (read(pipefds[0], &buf, 1) > 0) {
        printf("%c", buf);
    }
    // NOTE: process ends on SIGUSR1
    while (1) { sleep(100); }
    exit(EXIT_FAILURE);
}


/* Program Entry */

int main(void) {

    // initialize pipe
    while (pipe(pipefds) == -1) ;

    pid_t pid;

    // create child processes 1
    while ((pid = fork()) == -1) ;
    if (pid == 0) {
        child_1_fn();
        exit(EXIT_FAILURE);
    } else {
        pid_pool[0] = pid;
    }

    // create child process 2
    while ((pid = fork()) == -1) ;
    if (pid == 0) {
        child_2_fn();
        exit(EXIT_FAILURE);
    } else {
        pid_pool[1] = pid;
    }

    // set main process signal handler
    signal(SIGINT, main_signal_handler);

    // sleep indefinitely, process ends on SIGINT
    while (1) { sleep(100); }

    exit(EXIT_FAILURE);
}

