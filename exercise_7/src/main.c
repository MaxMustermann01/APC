/*********************************************************************************
 * FILENAME         main.c
 * 
 * DESCRIPTION      These functions are part of the submission to exercises of 
 *                  the "Advanced Parallel Computing" 
 *                  lecture of the University of Heidelberg.
 * 
 *                  Exercise 7 - Red-Black Tree
 *                               
 * 
 * AUTHORS          Sven Dorkenwald
 *                  Günther Schindler
 *
 * LAST CHANGE      03. JUNE 2015
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

int main(int argc, char **argv) {
    int i, j;
    int iOperations; // Number of operations which should be performed
    int iNumAdd, iNumSearch; // Ratio between insert and search operations
    int iKey, iVal; // Variables for each key and value
    double dstart = 0.0, dend = 0.0;
    rbtree t = rbtree_create();
    
    /* Check integrity of arguments */
    if(argc == 2) {
        iOperations = atoi(argv[1]);
        iNumAdd = DEFAULT_ADD;
        iNumSearch = DEFAULT_SEARCH;
        printf("\n[INFO] Executing %d 0perations", iOperations);
        printf("\n[INFO] Using ratio %d/%d per stream", iNumAdd, iNumSearch);
    }
    else if(argc == 4) {
        iOperations = atoi(argv[1]);
        iNumAdd = atoi(argv[2]);
        iNumSearch = atoi(argv[3]);
        printf("\n[INFO] Executing %d 0perations", iOperations);
        printf("\n[INFO] Using ratio %d/%d per stream", iNumAdd, iNumSearch);
    }
    else {
        printf("\n[ERROR] Wrong number of arguments. Check usage!"
               " \nrbtree <NUM_OPERATIONS> (opt <NUM_ADD <NUM_SEARCH>)\n");
        exit(1);
    }

    /* Initialize the RB-Tree with index 0 to MAX_ELEMENTS */
    for(i=0; i<INIT_ELEMENTS; i++) {
        iKey = rand(); // index 
        iVal = rand(); // random value between 0 and 10.000
#ifdef TRACE
        print_tree(t);
        printf("Inserting %d -> %d\n\n", iKey, iVal);
#endif
        rbtree_insert(t, (void*)iKey, (void*)iVal, compare_int);
    }
    
    /* Start time measurement */
    dstart = dstartMesGTOD();
  
    /* Generate the update stream */
    i = 0;
    while(i < iOperations) {
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
        for(j = 0; j < iNumAdd; j++) {
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
    
    /* Stop time measurement */
    dend = dstopMesGTOD(dstart);
    
    /* Evaluate execution time */
    printf("\n[INFO] Operations per second : %lf OP/s\n", i/dend);
    
    return 0;
}