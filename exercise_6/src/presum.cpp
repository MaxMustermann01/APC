//
// Created by sven on 02.05.15.
//

#include <pthread.h>
#include <iostream>
#include <sys/time.h>
#include <stdlib.h>
#include <fstream>
#include <stdint.h>
#include <math.h>
#include "time_measurement.h"

pthread_barrier_t barrier;

// Struct for passing pthread parameters
typedef struct sArg_t sArg_t;
struct sArg_t{
    int* array;
    int length;
    int id;
    int nThreads;
};

void* prefix_sum(void* argp){
    sArg_t *thread = (sArg_t*) argp;

    int d_stop = (int) log2(thread->length)-1;

// Upsweep --------------------------------------------------
    for (int d = 0; d <= d_stop; ++d){
    	int step = (int) pow(2, d+1);    
    	int last_step = (int) pow(2,d)-1; 
    	if (step*thread->id < thread->length - 1){
    		for (int k = step*thread->id; k < thread->length; k+=step*thread->nThreads) {
    			thread->array[k+step-1] += thread->array[k+last_step];
    		}
    	}
    	pthread_barrier_wait(&barrier);
    }

    if (thread->id == 0) thread->array[thread->length-1] = 0;
    int d_start = (int) log2(thread->length);
    int t;

    pthread_barrier_wait(&barrier);


// Downsweep -----------------------------------------------
    for (int d = d_start-1; d >= 0; --d){
    	int step = pow(2, d+1);     
    	int last_step = pow(2, d);
    	if (step*thread->id < thread->length - 1){
    		for (int k = step*thread->id; k < thread->length; k+=step*thread->nThreads) {
    			t = thread->array[k+last_step-1];
    			thread->array[k+last_step-1] = thread->array[k+step-1];
    			thread->array[k+step-1] += t;
    		}
    	}
    	pthread_barrier_wait(&barrier);
    }
    pthread_exit(NULL);
}


int main(int argc, char* argv[]){
    int nThreads, arrayLength;
    double dstart = 0.0, dend = 0.0;
    int num_runs = 20;
    double med_time = 0;

    if (argc == 3){
        arrayLength = atoi(argv[1]);
        nThreads = atoi(argv[2]);
    } else {
        std::cout << "[ERROR] Wrong input, expected <int arrayLength> <int nThreads>" << std::endl;
        exit(1);
    }

    double *times = new double[num_runs];
   	int *array = new int[arrayLength];
   	// int *copy_array = new int[arrayLength];
   	// srand(100);
   	// for (int ii = 0; ii < arrayLength; ++ii){
   		// array[ii] = rand()%10;
   		// copy_array[ii] = array[ii];
   		// std::cout << array[ii] << " ";
   	// }
   	// std::cout << "\n";

    sArg_t *arg = new sArg_t[nThreads];

// Pthread initilization
    void *status;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_t *threads = new pthread_t[nThreads];

    for (int nThread = 0; nThread < nThreads; ++nThread) {
        arg[nThread].id = nThread;
        arg[nThread].length = arrayLength;
        arg[nThread].array = array;
        arg[nThread].nThreads = nThreads;
    }

    if(pthread_barrier_init(&barrier, NULL, nThreads)){
        std::cout << "[ERROR] Could not create a barrier\n" << std::endl;
        exit(1);
    }

// Running the prefix sum multiple times
    for (int run_id=0; run_id<num_runs; ++run_id){
	    
    	srand(100+run_id);
	    for (int ii = 0; ii < arrayLength; ++ii){
   			array[ii] = rand()%10;
	    }

	    dstart = dstartMesGTOD();

		for (int nThread = 0; nThread < nThreads; ++nThread) {
	        pthread_create(&threads[nThread], &attr, &prefix_sum, (void *) &(arg[nThread]));
	    }
	    for (int nThread = 0; nThread < nThreads; ++nThread) {
	        pthread_join(threads[nThread], &status);
	    }

	    dend = dstopMesGTOD(dstart);
	    times[run_id] = dend;
	    med_time += dend/num_runs;
	}

/*    if (arrayLength <= 256){
	    std::cout << "\nRun test...\n";
	    std::cout << "\nWAS | IS | SHOULD\n";
	    for (int ii = 0; ii < arrayLength; ++ii){
	    	std::cout << copy_array[ii] << " ";
	    	if (ii > 0){
	    		copy_array[ii] += copy_array[ii-1];
	    	}
	    	std::cout << array[ii] << " " << copy_array[ii] << "\n";
	    }
	}*/
/*
    std::cout << "\n";
    std::cout << "----------------------------------------------------\n";
    std::cout << "Number of Threads: " << nThreads << "\n";
    std::cout << "Time in total[s]: " << med_time << "\n";
*/  
// Printing for shellscript output
	std::cout << nThreads << " " << med_time << "\n";  

	delete[] array;

    return 0;

}