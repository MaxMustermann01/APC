//
// Created by sven on 02.05.15.
//

#include <pthread.h>
#include <iostream>
#include <sys/time.h>
#include <stdlib.h>
#include <fstream>

struct sArrayArg {
    int nItems;
    int* fData;
    int* tData;
    int id;
};

void* loadArray(void* argp){
    struct sArrayArg* arg = (sArrayArg*) argp;
    volatile int *tData = arg->tData;
    volatile int *fData = arg->fData;
    int nItems = arg->nItems;
    int id = arg->id;

    for (int i = 0; i < nItems; ++i) {
        tData[i] = fData[id*nItems+i];
    }
    pthread_exit(NULL);
}

int main(int argc, char* argv[]){
    int size = 1000;
    int mThreads = 40;

    if (argc == 3){
        size = atoi(argv[1]);
        mThreads = atoi(argv[2]);
    }

    double* bandwidthArray = new double[mThreads];

    for (int nThreads = 1; nThreads <= mThreads; ++nThreads) {
        double tTime = 0;
        int nIterations = 0;

        while (tTime < 5) {
            int *fAry = new int[size]; // ----
            int *tAry = new int[size/nThreads]; // ----

            sArrayArg *arg = new sArrayArg[nThreads];
            void *status;
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
            pthread_t *threads = new pthread_t[nThreads];

            for (int nThread = 0; nThread < nThreads; ++nThread) {
                arg[nThread].fData = fAry;//[nThread];
                arg[nThread].tData = tAry;
                arg[nThread].nItems = size/nThreads; // ----
                arg[nThread].id = nThread;
            }

            timeval start, end;
            gettimeofday(&start, 0);
            double t1 = start.tv_sec + (start.tv_usec / 1000000.0);

            for (int nThread = 0; nThread < nThreads; ++nThread) {
                pthread_create(&threads[nThread], &attr, &loadArray, (void *) &(arg[nThread]));
            }
            for (int nThread = 0; nThread < nThreads; ++nThread) {
                pthread_join(threads[nThread], &status);
            }

            gettimeofday(&end, 0);
            double t2 = end.tv_sec + (end.tv_usec / 1000000.0);
            tTime = tTime + t2 - t1;

            delete[] threads;
            delete[] fAry;
            delete[] tAry;
            nIterations++;
        }
        double aTime = tTime/nIterations;
        std::cout << tTime << " " << nIterations << " " << nThreads << "\n";
        double bandwidthGB = sizeof(int)*size/aTime/1000000000; // ----
        bandwidthArray[nThreads-1] = bandwidthGB;
    }

    std::ofstream fout("bandwidths_e330.txt");
    if (fout.is_open()){
        for(int i = 0; i < mThreads; ++i){
            fout << i+1 << " " << bandwidthArray[i] << "\n";
        }
        fout.close();
    }
    else std::cout << "Unable to open file";
    delete[] bandwidthArray;
//    std::cout << "\n";
//    std::cout << "----------------------------------------------------\n";
//    std::cout << "Number of Threads: " << nThreads << "\n";
//    std::cout << "Size[Byte]: " << size* sizeof(int) << "\n";
//    std::cout << "Time in total[s]: " << tTime << "\n";
//    std::cout << "Number of Iterations: " << nIterations << "\n";
//    std::cout << "Time per iteration[s]: " << aTime << "\n";
//    std::cout << "Bandwidth[GB/s]: " << bandwidthGB << "\n";
//    std::cout << "----------------------------------------------------\n";
//    std::cout << "\n";

    return 0;
}