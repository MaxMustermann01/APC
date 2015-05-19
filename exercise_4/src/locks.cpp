//
// Created by sven on 02.05.15.
//

#include <pthread.h>
#include <iostream>
#include <sys/time.h>
#include <stdlib.h>
#include <fstream>
#include <stdint.h>
#include "time_measurement.h"


pthread_barrier_t barrier;
int counter;

/* Atomic exchange (of various sizes) */
static inline void *xchg_64(void *ptr, void *x)
{
    __asm__ __volatile__("xchgq %0,%1"
    :"=r" ((unsigned long long) x)
    :"m" (*(volatile long long *)ptr), "0" ((unsigned long long) x)
    :"memory");

    return x;
}

#define barrier() asm volatile("": : :"memory")
#define cpu_relax() asm volatile("pause\n": : :"memory")
#define cmpxchg(P, O, N) __sync_val_compare_and_swap((P), (O), (N))

typedef struct sQnode_t sQnode_t;
struct sQnode_t {
    struct sQnode_t *next;
    int locked;
};

typedef struct sQnode_t *lock;
lock global_lock;

static void acquire_lock(lock *l, sQnode_t *thisQnode){
    sQnode_t *prenode;

    thisQnode->next = NULL;
    thisQnode->locked = 1;

    prenode = (sQnode_t*) xchg_64(l, thisQnode);

    if (!prenode) return;

    prenode->next = thisQnode;

    barrier();

    while (thisQnode->locked) cpu_relax();

    return;
}

static void release_lock(lock *l, sQnode_t *thisQnode){
    if (!thisQnode->next){
        if (cmpxchg(l, thisQnode, NULL) == thisQnode) return;

        while (!thisQnode->next) cpu_relax();
    }
    thisQnode->next->locked = 0;
}

typedef struct sArg_t sArg_t;
struct sArg_t{
    int counts;
    int id;
};

void vInc(int *counter){
    (*counter)++;
}

void* countMSC(void* argp){
    sArg_t *thread = (sArg_t*) argp;

    pthread_barrier_wait(&barrier);
    sQnode_t *qnode = new sQnode_t;

    for (int ii = 0; ii < thread->counts; ++ii){
        acquire_lock(&global_lock, qnode);

        vInc(&counter);

        release_lock(&global_lock, qnode);
    }

    pthread_exit(NULL);
}

int main(int argc, char* argv[]){
    counter = 0;
    int nThreads, mCounter;
    double dstart = 0.0, dend = 0.0;

    if (argc == 3){
        mCounter = atoi(argv[1]);
        nThreads = atoi(argv[2]);
    } else {
        std::cout << "[ERROR] Wrong input, expected <int C> <int N>" << std::endl;
        exit(1);
    }

    sArg_t *arg = new sArg_t[nThreads];
    // double tTime = 0;

    void *status;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_t *threads = new pthread_t[nThreads];

    for (int nThread = 0; nThread < nThreads; ++nThread) {
        arg[nThread].id = nThread;
        arg[nThread].counts = 0;
    }

    for (int iC = 0; iC < mCounter; ++iC) {
        arg[iC % nThreads].counts += 1;
    }

    if(pthread_barrier_init(&barrier, NULL, nThreads)){
        std::cout << "[ERROR] Could not create a barrier\n" << std::endl;
        exit(1);
    }

//    timeval start, end;
//    gettimeofday(&start, 0);
//    double t1 = start.tv_sec + (start.tv_usec / 1000000.0);

//    std::cout << "Start threads" << std::endl;

    dstart = dstartMesGTOD();

    for (int nThread = 0; nThread < nThreads; ++nThread) {
        pthread_create(&threads[nThread], &attr, &countMSC, (void *) &(arg[nThread]));
    }
    for (int nThread = 0; nThread < nThreads; ++nThread) {
        pthread_join(threads[nThread], &status);
    }

//    gettimeofday(&end, 0);
//    double t2 = end.tv_sec + (end.tv_usec / 1000000.0);
//    tTime = tTime + t2 - t1;
    dend = dstopMesGTOD(dstart);

    if (counter != mCounter){
        std::cout << "[ERROR] something went wrong...\n";
        exit(1);
    }

    std::cout << "\n";
    std::cout << "----------------------------------------------------\n";
    std::cout << "Counter value: " << counter << "\n";
    std::cout << "Number of Threads: " << nThreads << "\n";
    std::cout << "Time in total[s]: " << dend << "\n";

    std::cout << nThreads << " " << dend << " " <<  counter/dend << "\n";

    return 0;
}