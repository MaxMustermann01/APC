/*********************************************************************************
 * FILENAME         main.c
 * 
 * DESCRIPTION      These functions are part of the submission to exercises of 
 *                  the "Advanced Parallel Computing" 
 *                  lecture of the University of Heidelberg.
 * 
 *                  Exercise 8 - Red-Black Tree
 *                               
 * 
 * AUTHORS          Sven Dorkenwald
 *                  Günther Schindler
 *
 * LAST CHANGE      12. JUNE 2015
 * 
 ********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <assert.h>
#include "rb_tree.h"
#include "time_measurement.h"

/* Uncommend this line if you want to trace and print the tree */
//#define TRACE
#define INIT_ELEMENTS 10000000
#define DEFAULT_ADD 10
#define DEFAULT_SEARCH 90

int iOperations; // Number of operations which should be performed
int iN; // Thread count
int iNumAdd, iNumSearch; // Ratio between insert and search operations
int iKey, iVal; // Variables for each key and value
rbtree t; // RB-Tree

void vUpdateStream(void *arg){
    int i, j, iID, iOpsPerThread;
    iID = (intptr_t) arg;
    /* Calculate how many Operations each thread should performe */
    iOpsPerThread = ((iID+1)*iOperations)/iN - (iID*iOperations)/iN;
    /* Generate the update stream */
    i = 0;
    while(i < iOpsPerThread) {
        /* Insert operations */
        for(j = 0; j < iNumAdd; j++) {
            i++;
            iKey = rand();
            iVal = rand();
#ifdef TRACE
            print_tree(t);
            printf("Inserting %d -> %d\n\n", iKey, iVal);
#endif
            rbtree_insert(t, (void*)iKey, (void*)iVal, compare_int);
        }
        /* Search operations */
        for(j = 0; j < iNumSearch; j++) {
            i++;
            iKey = rand();
            iVal = rand();
            rbtree_lookup(t, (void*)iKey, compare_int);
#ifdef TRACE
            print_tree(t);
            printf("Searching %d -> %d\n\n", iKey, rbtree_lookup(t, (void*)iKey, compare_int));
#endif
        }
    }
}

int main(int argc, char **argv) {
    int i, k, l;
    double dstart = 0.0, dend = 0.0;
    pthread_t *threads; // array of pThreads
    int threadnum[8] = {2, 4, 8, 12, 16, 24, 32, 48};
    int datarray[7] = {100000, 500000, 1000000, 5000000, 10000000, 25000000, 50000000};
    char name[16];
    FILE *fp;
    
    /* Check integrity of arguments */
    /*if(argc == 3) {
        iOperations = atoi(argv[1]);
	iN = atoi(argv[2]);
        iNumAdd = DEFAULT_ADD;
        iNumSearch = DEFAULT_SEARCH;
        printf("\n[INFO] Executing %d 0perations", iOperations);
        printf("\n[INFO] Using ratio %d/%d per stream", iNumAdd, iNumSearch);
	printf("\n[INFO] Using %d threads", iN);
    }
    else if(argc == 5) {
        iOperations = atoi(argv[1]);
	iN = atoi(argv[2]);
        iNumAdd = atoi(argv[3]);
        iNumSearch = atoi(argv[4]);
        printf("\n[INFO] Executing %d 0perations", iOperations);
        printf("\n[INFO] Using ratio %d/%d per stream", iNumAdd, iNumSearch);
	printf("\n[INFO] Using %d threads", iN);
    }
    else {
        printf("\n[ERROR] Wrong number of arguments. Check usage!"
               " \nrbtree <NUM_OPERATIONS> <NUM_THREADS> (opt <NUM_ADD <NUM_SEARCH>)\n");
        exit(1);
    }*/
    
    /* Initialize the read-write lock */
    if(pthread_rwlock_init(&rwlock, NULL)){
        printf("\n[ERROR] Could not create read-write lock");
        exit(1);
    }
    
    /* Create the RB-Tree */
    t = rbtree_create();
    
    /* Initialize the RB-Tree with MAX_ELEMENTS (10M) */
    for(i=0; i<INIT_ELEMENTS; i++) {
        iKey = rand(); // index 
        iVal = rand(); // random value between 0 and 10.000
#ifdef TRACE
        print_tree(t);
        printf("Inserting %d -> %d\n\n", iKey, iVal);
#endif
        rbtree_insert(t, (void*)iKey, (void*)iVal, compare_int);
    }
    
    // iterate all thread counts
    for(k = 0; k < 8; k++){
      
    iN = threadnum[k];
    
    /* Allocate memory for the thread-pointer */
    threads = (pthread_t *) malloc(iN*sizeof(pthread_t));
    
    // iterate all ratios
    for(k = 0; k<3; k++){
      
    if(k==0){
      iNumAdd = 10;
      iNumSearch = 90;
    }
    else if(k==1){
      iNumAdd = 50;
      iNumSearch = 50;
    }
     else if(k==2){
      iNumAdd = 90;
      iNumSearch = 10;
    }
    
    sprintf(name, "res_%d_%d_%d.dat",iNumAdd,iNumSearch,iN);
    fp=fopen(name, "w+");
    
    if(fp==NULL){
      printf("[ERROR] Could not open file");
      exit(1);
    }
    
    // iterate all data arrays
    for(j = 0; j<7; j++){
      
    iOperations = datarray[j];
    
    // stats
    printf("\n[INFO] Executing %d 0perations", iOperations);
    printf("\n[INFO] Using ratio %d/%d per stream", iNumAdd, iNumSearch);
    printf("\n[INFO] Using %d threads", iN);
    
    /* Start time measurement */
    dstart = dstartMesGTOD();
    
    /* Generate the update stream */
    for(i = 0; i < iN; ++i){
        if(pthread_create(&threads[i], NULL, (void*) vUpdateStream, (void *) (intptr_t)i)){
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
    
    /* Destroy lock */
    if(pthread_rwlock_destroy(&rwlock)){
        printf("\n[ERROR] Could not destroy read-write lock");
        exit(1);
    }
    
    /* Evaluate execution time */
    printf("\n[INFO] Operations per second : %lf OP/s\n", iOperations/dend);
    fprintf(fp, "%d %d\n", iOperations, (iOperations/dend)/10e3);
    } // data iteration
    fclose(fp);
    } // ratio iteration
    } // thread iteration
    
    return 0;
}