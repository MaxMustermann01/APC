/*********************************************************************************
 * FILENAME         main.c
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
 * LAST CHANGE      11. MAY 2015
 * 
 ********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include "count.h"
#include "time_measurement.h"

int main(int argc, char **argv) {
  
  int i; 		// control variable
  double dstart = 0.0, dend = 0.0;
  pthread_t *threads;	// array of pThreads
  
  /* Check integrity of arguments */
  if(argc!=3)
  {
    printf("\n[ERROR] Wrong number of arguments. Check usage!"
           " \ncounter <NUM_INC> <NUM_THREADS>\n");
    exit(1);
  }
  
  /* Convert arguments to int */
  iC = atoi(argv[1]);
  iN = atoi(argv[2]);
  
  /* Allocate memory for the thread-pointer */
  threads = (pthread_t *) malloc(iN*sizeof(pthread_t));
  
  /* Barrier initialization */
  if(pthread_barrier_init(&barrier, NULL, iN)){
    printf("[ERROR] Could not create a barrier\n");
    exit(1);
  }
  
  /************************************* Mutex *************************************/
  /* Set count variable to zero */
  iCount = 0;
  
  /* Start time measurement */
  dstart = dstartMesGTOD();

  /* Mutex initialization */
  if(pthread_mutex_init(&mutex, NULL)){
    printf("[ERROR] Could not create a mutex\n");
    exit(1);
  }
  
  /* Create all threads to count with the mutex */
  for(i = 0; i < iN; ++i){
    if(pthread_create(&threads[i], NULL, (void*) vCountMutex, (void *) (intptr_t)i)){
      printf("[ERROR] Could not create thread\n");
      exit(1);
    }
  }
  
  /* Finally join all threads */
  for(i = 0; i < iN; ++i){
    if(pthread_join(threads[i], NULL)){
      printf("[ERROR] Could not join thread\n");
      exit(1);
    }
  }
  
  /* Stop time measurement */
  dend = dstopMesGTOD(dstart);
  
  /* Check if count was a success */
  if(!(iCount == iC))
    printf("[ERROR] Count [Mutex] failed: %d != %d\n", iCount, iC);
  else{
    printf("[INFO] Count [Mutex] succeeded: %d = %d\n", iCount, iC);
    printf("[INFO] Elapsed time [Mutex] : %lfms\n", dend*10e3);
    printf("[INFO] Updates [Mutex] : %.2lf * 10^6/s\n", iC/dend/10e6);
  }
  
  /****************************** Atomic Operation **********************************/
  /* Set count variable to zero */
  iCount = 0;
  
  /* Start time measurement */
  dstart = dstartMesGTOD();
  
  /* Create all threads to count with the atomic operation */
  for(i = 0; i < iN; ++i){
    if(pthread_create(&threads[i], NULL, (void*) vCountAtom, (void *) (intptr_t)i)){
      printf("[ERROR] Could not create thread\n");
      exit(1);
    }
  }
  
  /* Finally join all threads */
  for(i = 0; i < iN; ++i){
    if(pthread_join(threads[i], NULL)){
      printf("[ERROR] Could not join thread\n");
      exit(1);
    }
  }
  
  /* Stop time measurement */
  dend = dstopMesGTOD(dstart);
  
  /* Check if count was a success */
  if(!(iCount == iC))
    printf("[ERROR] Count [AtomOp] failed: %d != %d\n", iCount, iC);
  else{
    printf("[INFO] Count [AtomOp] succeeded: %d = %d\n", iCount, iC);
    printf("[INFO] Elapsed [AtomOp] time : %lfms\n", dend*10e3);
    printf("[INFO] Updates [AtomOp] : %.2lf * 10^6/s\n", iC/dend/10e6);
  }
  
  /****************************** Read Modify Write ********************************/
  /* Set count variable to zero */
  iCount = 0;
  
  /* Start time measurement */
  dstart = dstartMesGTOD();
  
  initlock_rmw(&lock);
  
  /* Create all threads to count with the atomic operation */
  for(i = 0; i < iN; ++i){
    if(pthread_create(&threads[i], NULL, (void*) vCountRMW, (void *) (intptr_t)i)){
      printf("[ERROR] Could not create thread\n");
      exit(1);
    }
  }
  
  /* Finally join all threads */
  for(i = 0; i < iN; ++i){
    if(pthread_join(threads[i], NULL)){
      printf("[ERROR] Could not join thread\n");
      exit(1);
    }
  }
  
  /* Stop time measurement */
  dend = dstopMesGTOD(dstart);
  
  /* Check if count was a success */
  if(!(iCount == iC))
    printf("[ERROR] Count [RMW] failed: %d != %d\n", iCount, iC);
  else{
    printf("[INFO] Count [RMW] succeeded: %d = %d\n", iCount, iC);
    printf("[INFO] Elapsed [RMW] time : %lfms\n", dend*10e3);
    printf("[INFO] Updates [RMW] : %.2lf * 10^6/s\n", iC/dend/10e6);
  }
  
  return 0;
}