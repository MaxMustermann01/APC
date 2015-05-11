/*********************************************************************************
 * FILENAME         count.c
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
#include "count.h"
#include <pthread.h>
#include <stdint.h>

static void lock_rmw(lock_t *);

static void unlock_rmw(lock_t *);

void vInc(int *iCnt){
  (*iCnt)++;
}

void vCountMutex(void *arg){
  int i, iID;
  iID = (intptr_t) arg;
  /* Synchronize threads */
  pthread_barrier_wait(&barrier);
  /* Start incrementation process */
  for(i = (iID*iC)/iN; i < ((iID+1)*iC)/iN; i++){
    /* Lock mutex */
    pthread_mutex_lock(&mutex);
    /* Increment count variable */
    vInc(&iCount);
    /* Unlock mutex */
    pthread_mutex_unlock(&mutex);
  }
}

void vCountAtom(void *arg){
  int i, iID;
  iID = (intptr_t) arg;
  /* Synchronize threads */
  pthread_barrier_wait(&barrier);
  /* Start incrementation process */
  for(i = (iID*iC)/iN; i < ((iID+1)*iC)/iN; i++){
    /* Increment count variable */
    __sync_add_and_fetch(&iCount,1);
  }
}

void initlock_rmw(lock_t *t){
  t->ticket = 1;
  t->users = 0;
}

static void lock_rmw(lock_t *t){
  int me = __sync_add_and_fetch(&(t->users), 1);
  while (t->ticket != me)
    cpu_relax();
}

static void unlock_rmw(lock_t *t){
  __sync_add_and_fetch(&(t->ticket), 1);
}

void vCountRMW(void *arg){
  int i, iID;
  iID = (intptr_t) arg;
  /* Synchronize threads */
  pthread_barrier_wait(&barrier);
  /* Start incrementation process */
  for(i = (iID*iC)/iN; i < ((iID+1)*iC)/iN; i++){
    /* Lock */
    lock_rmw(&lock);
    /* Increment count variable */
    vInc(&iCount);
    /* Unlock */
    unlock_rmw(&lock);
  }
}