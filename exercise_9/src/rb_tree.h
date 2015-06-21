/*********************************************************************************
 * FILENAME         rb_tree.h
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
#include <pthread.h>

/* Uncommend this line for verification */
//#define VERIFY_RBTREE
#define INDENT_STEP  4
#define FALSE   0
#define TRUE    1
//#define DEBUG

enum rbtree_node_color { RED, BLACK };

typedef struct rbtree_node_t {
    void* key;
    void* value;
    struct rbtree_node_t* left;
    struct rbtree_node_t* right;
    struct rbtree_node_t* parent;
    enum rbtree_node_color color;
    pthread_rwlock_t* rwlock;
} *rbtree_node;

typedef struct rbtree_t {
    rbtree_node root;
} *rbtree;

/* Forward declaration of compare function. You can overide this with int for example */
typedef int (*compare_func)(void* left, void* right);
int compare_int(void* left, void* right);

/* Create operation */
rbtree rbtree_create();

/* Search operation */
void* rbtree_lookup(rbtree t, void* key, compare_func compare);

/* Insert operation */
void rbtree_insert(rbtree t, void* key, void* value, compare_func compare, int tid);

void rbtree_insert_ser(rbtree t, void* key, void* value, compare_func compare, int tid);

/* Print tree */
void print_tree(rbtree t);
