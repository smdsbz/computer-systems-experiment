#include <stdio.h>
#include <stdlib.h>

// signal lights utilities
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                (Linux-specific) */
};

// POSIX thread utilities
#include <pthread.h>

// sleep()
#include <unistd.h>


/* Configurations */

#define PRODUCE_SPEED_RATIO ( 3 )
#define SLEEP_MAX_MILLISEC  ( 10 )

/* Global Shared Variables */

int common_a = 0;


/* Helper Functions: Synchronization */

/**
 * P Operation
 * :param semid: id of semaphore
 * :param idx: index of the target signal light in the semaphore group
 */
void P(int semid, int idx) {
    struct sembuf sem;
    sem.sem_num = idx;
    sem.sem_op = -1;    // mutex - 1
    sem.sem_flg = 0;
    semop(
        /*semid=*/semid,
        /*sops=*/&sem,
        /*nsops=*/1
    );
    return;
}

/**
 * V Operation
 * :param semid: id of the semaphore
 * :param idx: index of the target signal light in the semaphore group
 */
void V(int semid, int idx) {
    struct sembuf sem;
    sem.sem_num = idx;
    sem.sem_op = +1;    // mutex + 1
    sem.sem_flg = 0;
    semop(
        /*semid=*/semid,
        /*sops=*/&sem,
        /*nops=*/1
    );
    return;
}


/* Thread Jobs */

typedef struct _thread_1_args_t {
    int    *ptr_a;
    int     semid;
} thread_1_args_t;

void *thread_1_fn(void *args) {
    thread_1_args_t *params = (thread_1_args_t *)args;
    for (int val = 1; val <= 100; ++val) {
        P(params->semid, 0);    // get write lock
        *params->ptr_a += val;
        V(params->semid, 1);    // release read lock
        // NOTE: sleep for random amount of time, in order to prove the
        // integrity of the P-V action
        usleep((rand() % 1000) * PRODUCE_SPEED_RATIO * SLEEP_MAX_MILLISEC);
    }
    return NULL;
}


typedef thread_1_args_t thread_2_args_t;

void *thread_2_fn(void *args) {
    thread_2_args_t *params = (thread_2_args_t *)args;
    while (1) {
        P(params->semid, 1);    // get read lock
        printf("Current value: %d\n", *params->ptr_a);
        if (*params->ptr_a == 5050) {
            V(params->semid, 0);    // release write lock
            break;
        }
        V(params->semid, 0);    // release write lock
        usleep((rand() % 1000) * SLEEP_MAX_MILLISEC);
    }
    return NULL;
}


/* Program Entry */

int main(void) {

    srand(0);   // fix output

    // create semaphore group
    int semid = semget(
        /*key=*/IPC_PRIVATE,
        /*nsems=*/2,
        /*semflg=*/IPC_CREAT | 0666
    );
    if (semid == -1) { exit(EXIT_FAILURE); }

    // initialize semaphore
    union semun semarg;
    // sem[0] -- write semaphore
    semarg.val = 1;
    int retval = semctl(
        /*semid=*/semid,
        /*semnum=*/0,
        /*cmd=*/SETVAL,
        /*arg=*/semarg
    );
    // sem[1] -- read semaphore
    semarg.val = 0;
    retval = semctl(
        /*semid=*/semid,
        /*semnum=*/1,
        /*cmd=*/SETVAL,
        /*arg=*/semarg
    );

    // prepare parameters
    thread_1_args_t thread_1_args;
    thread_1_args.ptr_a = &common_a;
    thread_1_args.semid = semid;
    thread_2_args_t thread_2_args;
    thread_2_args.ptr_a = &common_a;
    thread_2_args.semid = semid;
    // create & start thread jobs
    pthread_t thread_1, thread_2;
    pthread_create(&thread_1, NULL, thread_1_fn, &thread_1_args);
    pthread_create(&thread_2, NULL, thread_2_fn, &thread_2_args);

    // wait for threads to exit
    pthread_join(thread_1, NULL);
    pthread_join(thread_2, NULL);

    printf("\n");

    return 0;
}

