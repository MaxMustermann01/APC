/*********************************************************************************
 * FILENAME         main.c
 * 
 * DESCRIPTION      These functions are part of the submission to exercises of 
 *                  the "Advanced Parallel Computing" 
 *                  lecture of the University of Heidelberg.
 * 
 *                  Exercise 5 - Barrier
 *                               
 * 
 * AUTHORS          Sven Dorkenwald
 *                  Günther Schindler
 *
 * LAST CHANGE      25. MAY 2015
 * 
 ********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include "barrier.h"
#include "time_measurement.h"

int main(int argc, char **argv) {
  
  int i; 		// control variable
  double dstart = 0.0, dend = 0.0;
  pthread_t *threads;	// array of pThreads
  
  /* Check integrity of arguments */
  if(argc!=3)
  {
    printf("\n[ERROR] Wrong number of arguments. Check usage!"
           " \nbarrier <NUM_ITER> <NUM_THREADS>\n");
    exit(1);
  }
  
  /* Convert arguments to int */
  iIter = atoi(argv[1]);
  iN = atoi(argv[2]);
  
  /* Allocate memory for the thread-pointer */
  threads = (pthread_t *) malloc(iN*sizeof(pthread_t));
  
  /***************************** PThread Barrier *********************************/
  /* Barrier initialization */
  if(pthread_barrier_init(&barrier, NULL, iN)){
    printf("[ERROR] Could not create a barrier\n");
    exit(1);
  }
  
  /* Start time measurement */
  dstart = dstartMesGTOD();
  
  /* Create all threads to count with the mutex */
  for(i = 0; i < iN; ++i){
    if(pthread_create(&threads[i], NULL, (void*) vBarrierPThread, NULL)){
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
  
  /* Evaluate execution time */
  printf("[INFO] Barrier Latency [PThread] : %lfus\n", (dend/iIter)*10e6);
  
  /***************************** Central Barrier *********************************/
  /* Barrier initialization */
  init_central_barrier();
  
  /* Start time measurement */
  dstart = dstartMesGTOD();
  
  /* Create all threads to count with the mutex */
  for(i = 0; i < iN; ++i){
    if(pthread_create(&threads[i], NULL, (void*) vBarrierCentral, NULL)){
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
  
  /* Evaluate execution time */
  printf("[INFO] Barrier Latency [Central] : %lfus\n", (dend/iIter)*10e6);
  
 /**************************** Tournament Barrier *******************************/
  
  /* Initialize tournament barrier */
  rounds = initializeTournamentCommonStructures(iN);
  
  /* Start time measurement */
  dstart = dstartMesGTOD();
  
  /* Create all threads */
  for(i = 0; i < iN; ++i){
    if(pthread_create(&threads[i], NULL, (void*) vBarrierTournament, (void *) (intptr_t)i)){
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
  
  /* Evaluate execution time */
  printf("[INFO] Barrier Latency [Tournament] : %lfus\n", (dend/iIter)*10e6);
  
/**************************** Dissemination Barrier*****************************/
  
  /* Initialize dissemination barrier */
  procNodes = initializeDisseminationCommonStructures(iN);
  
  /* Start time measurement */
  dstart = dstartMesGTOD();
  
  /* Create all threads */
  for(i = 0; i < iN; ++i){
    if(pthread_create(&threads[i], NULL, (void*) vBarrierDissemination, (void *) (intptr_t)i)){
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
  
  /* Evaluate execution time */
  printf("[INFO] Barrier Latency [Dissemination] : %lfus\n", (dend/iIter)*10e6);
  
/************************************ Tree Barrier *******************************/
  
  /* Initialize tree barrier */
  nodes = initializeMCSCommonStructures(iN);
  
  /* Start time measurement */
  dstart = dstartMesGTOD();
  
  /* Create all threads */
  for(i = 0; i < iN; ++i){
    if(pthread_create(&threads[i], NULL, (void*) vBarrierDissemination, (void *) (intptr_t)i)){
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
  
  /* Evaluate execution time */
  printf("[INFO] Barrier Latency [Tree] : %lfus\n", (dend/iIter)*10e6);
  
  return 0;
}