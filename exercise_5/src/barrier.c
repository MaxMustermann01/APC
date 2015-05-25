/*********************************************************************************
 * FILENAME         barrier.c
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
#include "barrier.h"
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

unsigned int ceil_log2( unsigned  int n ){
    return ceil( log( (double)n ) / log( 2 ));  
}

void vBarrierPThread(void *arg){
  int i;
  /* Start barrier loop */
  for(i = 0; i < iIter; i++){
    pthread_barrier_wait(&barrier);
  }
}

void init_central_barrier(void){
  /* Set count variable to thread count */
  iCount = iN;
  /* Set sense initial to true */
  iSense = true;
}

void central_barrier(int *local_sense){
  /* Each thread toggles it's own sense */
  *local_sense = 1 - *local_sense;
  if(__sync_sub_and_fetch(&iCount, 1) == 0){
    iCount = iN;
    /* last thread toggles global sense */
    iSense = *local_sense;
  }
  else
    /* spin wait */
    while(iSense != *local_sense); 
}

void vBarrierCentral(void *arg){
  int local_sense = true;
  int i;
  /* Start barrier loop */
  for(i = 0; i < iIter; i++){
    central_barrier(&local_sense);
  }
}

void vBarrierTournament(void *arg){
  int i, iID, logiN;
  char local_sense = true;
  iID = (intptr_t) arg;
  
  /* Get log d from thread count */
  logiN = ceil_log2(iN);

  /* Start barrier loop */
  for(i = 0; i < iIter; i++){
    tournament_barrier(rounds, iID, logiN, &local_sense);
  }
}

void vBarrierDissemination(void *arg){
  ProcNode* currNode;
  int i, iID, logiN;
  char local_sense = true;
  int parity = 0;
  iID = (intptr_t) arg;
  
  /* Get log d from thread count */
  logiN = ceil_log2(iN);
  
  currNode = procNodes + iID;
  /* Start barrier loop */
  for(i = 0; i < iIter; i++){    
    dissemination_barrier(currNode, &local_sense, logiN, &parity);
  }
}

void vBarrierTree(void *arg){
  TreeNode* currNode;
  int i, iID;
  char local_sense = true;
  iID = (intptr_t) arg;
  
  currNode = nodes + iID;
  /* Start barrier loop */
  for(i = 0; i < iIter; i++){    
    mcs_barrier(currNode, iID, &local_sense);
  }
}

RoundNode** initializeTournamentCommonStructures(unsigned int no_of_threads) {
  int isStateSet;
  int i, k;
  RoundNode** rounds = NULL;
  if (no_of_threads > 0) {
    unsigned int logp = ceil_log2(no_of_threads);
    rounds = (RoundNode**) malloc(sizeof (RoundNode*) * no_of_threads);
    if (rounds) {
      memset(rounds, 0, sizeof (RoundNode*) * no_of_threads);
      for (i = 0; i < no_of_threads; i++) {
        rounds[i] = (RoundNode*) malloc(sizeof (RoundNode) * (logp + 1));
        if (rounds[i]) {
          memset(rounds[i], 0, sizeof (RoundNode)*(logp + 1));
        }
      }
      for (i = 0; i < no_of_threads; i++) {
        for (k = 0; k <= logp; k++) {
          RoundNode* currentRound = *(rounds + i) + k;
          currentRound->flag = 0;
          isStateSet = 0;
          if (k > 0) {
            if (!(i % (1 << k))) {
              if (((i + (1 << (k - 1))) < no_of_threads) && ((1 << k) < no_of_threads)) {
                currentRound->role = WINNER;
              isStateSet = 1;
             }
             else if ((i + (1 << (k - 1))) >= no_of_threads) {
               isStateSet = 1;
               currentRound->role = BYE;
              }
            }
            if (!isStateSet) {
              if ((i % (1 << k)) == (1 << (k - 1))) {
                currentRound->role = LOSER;
              } 
              else if (i == 0 && ((1 << k) >= no_of_threads)) {
                currentRound->role = CHAMPION;
              }
            }
          }
          else if (k == 0) {
            currentRound->role = DROPOUT;
          }
          RoundNode* toPoint = NULL;
          int temp;
          toPoint = NULL;
          switch (currentRound->role) {
            case LOSER:
              temp = i - (1 << (k - 1));
              toPoint = *(rounds + temp) + k;
              break;
            case WINNER:
            case CHAMPION:
              temp = i + (1 << (k - 1));
              toPoint = *(rounds + temp) + k;
              break;
            case BYE:
              break;
            case DROPOUT:
              break;
            }
            if (toPoint) {
              currentRound->opponent = &(toPoint->flag);
            }
          }
        }
      }
   }
    return rounds;
}

void tournament_barrier(RoundNode** rounds,int thread_no, int logp, char *sense) {
  int roundNo = 1;
  int needToExit = 0;
  if (logp > 0 && rounds) {
    while (1) {
      RoundNode* currNode = *(rounds + thread_no) + roundNo;
      needToExit = 0;
      switch (currNode->role) {
        case LOSER:
          *(currNode->opponent) = *sense;
          while (currNode->flag != *sense);
          needToExit = 1;
          break;
        case WINNER:
          while (currNode->flag != *sense);
          break;
        case CHAMPION:
          while (currNode->flag != *sense);
          *(currNode->opponent) = *sense;
          needToExit = 1;
          break;
        case BYE: break;
        case DROPOUT: break;
      }
      if (needToExit) {
        break;
      }
      roundNo++;
    }
    while (1) {
      roundNo--;
      RoundNode* currNode = *(rounds + thread_no) + roundNo;
      needToExit = 0;
      switch (currNode->role) {
        case LOSER: break;
        case WINNER:
          *(currNode->opponent) = *sense;
          break;
        case BYE: break;
        case CHAMPION: break;
        case DROPOUT:
          needToExit = 1;
        break;
      }
      if (needToExit) {
        break;
      }
    }
    *sense = !(*sense);
  }
}

ProcNode* initializeDisseminationCommonStructures(unsigned int no_of_threads) {
  ProcNode* allNodes = NULL;
  int allOk = 0;
  int i = 0;
  if (no_of_threads > 0) {
    unsigned int logp = ceil_log2(no_of_threads);
    allNodes = (ProcNode*) malloc(sizeof (ProcNode) * no_of_threads);
    if (allNodes) {
      for (i = 0; i < no_of_threads; i++) {
        allNodes[i].myflags[0] = (char*) malloc(sizeof (char) *logp);
        allNodes[i].myflags[1] = (char*) malloc(sizeof (char) *logp);
        if (!allNodes[i].myflags[0] || 
	  !allNodes[i].myflags[1]) {
        allOk = 1;
        break;
      }
      memset(allNodes[i].myflags[0], 0, logp);
      memset(allNodes[i].myflags[1], 0, logp);
      allNodes[i].partnerflags[0] = (char**) malloc(sizeof (char*) * logp);
      allNodes[i].partnerflags[1] = (char**) malloc(sizeof (char*) * logp);
      if (!allNodes[i].partnerflags[0] || !allNodes[i].partnerflags[1]) {
        allOk = 1;
        break;
      }
    }
    if (!allOk) {
      int r, k;
      for (i = 0; i < no_of_threads; i++) {
        for (k = 0; k < logp; k++) {
          for (r = 0; r < 2; r++) {
            *(allNodes[i].partnerflags[r] + k) = allNodes[(i + (1 << k)) % no_of_threads].myflags[r] + k;
           }
         }
       }
     }
   }
  }
  return allNodes;
}

void dissemination_barrier(ProcNode* localflags, char* sense, int logp, int* parity) {
  int i = 0;
  for (i = 0; i < logp; i++) {
    **(localflags->partnerflags[*parity] + i) = *sense;
    while (*(localflags->myflags[*parity] + i) != *sense);
  }
  if (*parity == 1) {
    *sense = !(*sense);
  }
  *parity = 1 - (*parity);
}

TreeNode* initializeMCSCommonStructures(unsigned int no_of_threads) {
    int i;
    int j = 0;
    TreeNode* nodes = NULL;
    if (no_of_threads > 0) {
        nodes = (TreeNode*) malloc(sizeof (TreeNode) * no_of_threads);
        if (nodes) {
            memset(nodes, 0, sizeof (TreeNode) * no_of_threads);
            for (i = 0; i < no_of_threads; i++) {
                for (j = 0; j < 4; j++) {
                    //This was the Bug in MCS paper
                    //in paper it is 4*i + j , where as it should be
                    // 4 * i + j + 1
                    nodes[i].haveChild[j] = (4 * i + j + 1) < no_of_threads;
                    nodes[i].childNotReady[j] = nodes[i].haveChild[j];
                }
                nodes[i].parentSense = 0;
                if (!i) {
                    nodes[i].parentPointer = &(nodes[i].dummy);
                } else {
#ifdef DEBUG1
                    printf("For Node:%d parent:%d,%d\n", i, (i - 1) / 4, (i - 1) % 4);
#endif
                    nodes[i].parentPointer = &(nodes[(i - 1) / 4].childNotReady[(i - 1) % 4]);
                }
                nodes[i].childPointers[0] = (2 * i + 1) >= no_of_threads ? &(nodes[i].dummy) : &(nodes[2 * i + 1].parentSense);
                nodes[i].childPointers[1] = (2 * i + 2) >= no_of_threads ? &(nodes[i].dummy) : &(nodes[2 * i + 2].parentSense);
                nodes[i].dummy = 1;
#ifdef DEBUG1
                printf("For Node:%d children:%d,%d\n", i, (2 * i + 1) >= no_of_threads ? i : (2 * i + 1), (2 * i + 2) >= no_of_threads ? i : (2 * i + 2));
#endif
            }
        }

    }
    return nodes;
}

void mcs_barrier(TreeNode* currNode, int curr_thread_no, char* sense) {
    int allChildrenReady = 0;
    int i = 0;
    do {
        allChildrenReady = 1;
        for (i = 0; i < 4; i++) {
            if (currNode->childNotReady[i]) {
                allChildrenReady = 0;
                break;
            }
        }

    } while (!allChildrenReady);

    for (i = 0; i < 4; i++) {
        currNode->childNotReady[i] = currNode->haveChild[i];
    }
#ifdef DEBUG1
    printf("Entering second loop:%d\n", curr_thread_no);
#endif
    *(currNode->parentPointer) = 0;
    if (curr_thread_no) {
        while (currNode->parentSense != *sense);
    }
    *(currNode->childPointers[0]) = *sense;
    *(currNode->childPointers[1]) = *sense;
    *sense = !(*sense);
}