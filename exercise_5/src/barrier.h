/*********************************************************************************
 * FILENAME         barrier.h
 * 
 * DESCRIPTION      These functions are part of the submission to exercises of 
 *                  the "Advanced Parallel Computing" 
 *                  lecture of the University of Heidelberg.
 * 
 *                  Exercise 5 - Shared Counter
 *                               
 * 
 * AUTHORS          Sven Dorkenwald
 *                  Günther Schindler
 *
 * LAST CHANGE      25. MAY 2015
 * 
 ********************************************************************************/
#include <pthread.h>

/* Pause instruction to prevent excess processor bus usage */ 
#define cpu_relax() asm volatile("pause\n": : :"memory")

#define false   0
#define true    1


int iIter, iN; 			// threads and count limit
int iCount;			// count variable
int iSense;			// sense variable
pthread_barrier_t barrier;
pthread_mutex_t mutex;
void vBarrierPThread(void *);

/********************************* For central barrier ***************************/

void vBarrierCentral(void *);

void init_central_barrier(void);

void central_barrier(int *);

/******************************* For tournament barrier **************************/
enum PossibleRoles{
  WINNER,
  LOSER,
  BYE,
  CHAMPION,
  DROPOUT
};
    
typedef struct round_node{
  enum PossibleRoles role;
  char* opponent;
  char flag;
} RoundNode;

RoundNode** rounds;

unsigned int ceil_log2(unsigned  int);

void vBarrierTournament(void *);

RoundNode** initializeTournamentCommonStructures(unsigned int no_of_threads);

void tournament_barrier(RoundNode** rounds,int thread_no, int logp, char *sense);

/**************************** For dissemination barrier **************************/

typedef struct flags{
  char* myflags[2];
  char** partnerflags[2];  
} ProcNode;

ProcNode* procNodes;

void vBarrierDissemination(void *);
    
ProcNode* initializeDisseminationCommonStructures(unsigned int no_of_threads);

void dissemination_barrier(ProcNode* localflags, char* sense, int logp, int* parity);

/*********************************** Tree barrier ********************************/

typedef struct mcs_node {
  char parentSense;
  char* parentPointer;
  char *childPointers[2];
  char haveChild[4];
  char childNotReady[4];
  char dummy;  
} TreeNode;

TreeNode* nodes;
    
TreeNode* initializeMCSCommonStructures(unsigned int no_of_threads);

void mcs_barrier(TreeNode* currNode, int curr_thread_no, char* sense);


