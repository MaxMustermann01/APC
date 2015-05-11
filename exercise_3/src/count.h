/*********************************************************************************
 * FILENAME         count.h
 * 
 * DESCRIPTION      These functions are part of the submission to exercises of 
 *                  the "Advanced Parallel Computing" 
 *                  lecture of the University of Heidelberg.
 * 
 *                  Exercise 3 - Shared Counter
 *                               
 * 
 * AUTHORS          Sven Dorkenwald
 *                  Günther Schindler
 *
 * LAST CHANGE      10. MAY 2015
 * 
 ********************************************************************************/
#include <pthread.h>

/* Pause instruction to prevent excess processor bus usage */ 
#define cpu_relax() asm volatile("pause\n": : :"memory")

typedef struct {
  int ticket;
  int users;
} lock_t;

int iC, iN; 			// threads and count limit
int iCount;			// count variable
pthread_barrier_t barrier;
pthread_mutex_t mutex;
lock_t lock;

void initlock_rmw(lock_t *);

void vInc(int *);

void vCountMutex(void *);

void vCountAtom(void *);

void vCountRMW(void *);