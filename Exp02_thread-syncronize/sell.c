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

#define NUM_WORKERS         ( 8 )
#define INIT_TICKET_COUNT   ( 10 )
#define SLEEP_MAX_MILLISEC  ( 10 )


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

typedef struct _seller_args_t {
    int     seller_id;
    int    *ptr_a;
    int     semid;
} seller_args_t;

void *seller_fn(void *args) {
    seller_args_t *params = (seller_args_t *)args;
    while (1) {
        P(params->semid, 0);
        // quit if no more tickets to sell
        if (*params->ptr_a == 0) {
            printf("[Thread %d] All sold out!\n", params->seller_id);
            V(params->semid, 0);
            return NULL;
        }
        // else, sell and print
        (*params->ptr_a)--;
        printf("[Thread %d] Sold! %d to go!\n", params->seller_id, *params->ptr_a);
        V(params->semid, 0);
        usleep((rand() % 1000) * SLEEP_MAX_MILLISEC);
    }
    exit(EXIT_FAILURE);
}


/* Program Entry */

int main(void) {

    srand(0);   // fix output

    int tickets_remain = INIT_TICKET_COUNT;

    // create semaphore group
    int semid = semget(
        /*key=*/IPC_PRIVATE,
        /*nsems=*/1,
        /*semflg=*/IPC_CREAT | 0666
    );
    if (semid == -1) { exit(EXIT_FAILURE); }

    // initialize semaphore
    union semun semarg;
    semarg.val = 1;
    int retval = semctl(
        /*semid=*/semid,
        /*semnum=*/0,
        /*cmd=*/SETVAL,
        /*arg=*/semarg
    );

    pthread_t thread_pool[NUM_WORKERS];
    seller_args_t args_pool[NUM_WORKERS];

    // create workers
    for (int idx = 0; idx != NUM_WORKERS; ++idx) {
        args_pool[idx].seller_id = idx + 1;
        args_pool[idx].ptr_a = &tickets_remain;
        args_pool[idx].semid = semid;
        pthread_create(&thread_pool[idx], NULL, seller_fn, &args_pool[idx]);
    }

    // wait for join
    for (int idx = 0; idx != NUM_WORKERS; ++idx) {
        pthread_join(thread_pool[idx], NULL);
    }

    /* printf("\n"); */

    return 0;
}


