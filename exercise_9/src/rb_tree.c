/*********************************************************************************
 * FILENAME         rb_tree.c
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
#include "rb_tree.h"
#include <assert.h>
#include <pthread.h>
#include <errno.h>

/***************************** Private type definitions ***************************/
/* Each node also stores its color, either red or black, using an enumeration */
typedef rbtree_node node;
typedef enum rbtree_node_color color;

/**************************** Private function prototypes *************************/
static node grandparent(node n);

static node sibling(node n);

static node uncle(node n);

static void verify_properties(rbtree t);

/* Each node is either red or black */
static void verify_property_1(node root);

/* The root node is black */
static void verify_property_2(node root);

/* All leaves are black and contain no data */
static color node_color(node n);

/* Every red node has two children, and both are black */
static void verify_property_4(node root);

/* All paths from any given node to its leaf nodes contain the same number of 
 * black nodes
 */
static void verify_property_5(node root);
static void verify_property_5_helper(node n, int black_count, int* black_count_path);

/* Allocate and initialize a new node */
static node new_node(void* key, void* value, color node_color, node left, node right);

static node lookup_node(rbtree t, void* key, compare_func compare);

/* Helper functions, one to perform a left rotation and one to perform a right 
 * rotation; each takes the highest node in the subtree as an argument */
static void rotate_left(rbtree t, node n);
static void rotate_right(rbtree t, node n);

 /* Helper function that cuts a node away from its parent, substituting a new 
  * node (or NULL) in its place */
 static void replace_node(rbtree t, node oldn, node newn);
 
/* Case 1, the new node is now the root node of the tree */
static void insert_case1(rbtree t, node n);

/* Case 2, the new node has a black parent */
static void insert_case2(rbtree t, node n);

/* Case 3, the uncle node is red */
static void insert_case3(rbtree t, node n);

/* Case 4, the new node is the right child of its parent and the parent is the left child 
 * of the grandparent. In this case we rotate left about the parent. 
 * Case 4, the new node is the left child of its parent and the parent is the right child of 
 * the grandparent. In this case we rotate right about the parent. */
static void insert_case4(rbtree t, node n);

/* Case 5, the new node is the left child of its parent and the parent is the left 
 * child of the grandparent. In this case we rotate right about the grandparent. 
 * Case 5, the new node is the right child of its parent and the parent is the right 
 * child of the grandparent. In this case we rotate left about the grandparent. */
static void insert_case5(rbtree t, node n);

/* Helper function to print tree */
void print_tree_helper(rbtree_node n, int indent);

/* Lock the 5 upper layers of n */
node wrlock_branch(node n, int);
/* Unlock the layers */
void unlock_branch(node lockednode, int);

/******************************** Node relationships ******************************/
node grandparent(node n) {
    if (n == NULL)
        return NULL;
    else if (n->parent == NULL)
        return NULL;
    else if (n->parent->parent == NULL)
        return NULL;
    else
        return n->parent->parent;
}

node sibling(node n) {
    if (n == NULL)
        return NULL;
    else if (n->parent == NULL)
        return NULL;
    else if (n == n->parent->left)
        return n->parent->right;
    else
        return n->parent->left;
}

node uncle(node n) {
    if (n == NULL)
        return NULL;
    else if (n->parent == NULL)
        return NULL;
    else if (sibling(n->parent) == NULL)
        return NULL;
    else
        return sibling(n->parent);
}

/**************************** Verify properties functions *************************/

void verify_properties(rbtree t) {
#ifdef VERIFY_RBTREE
    verify_property_1(t->root);
    verify_property_2(t->root);
    /* Property 3 is implicit */
    verify_property_4(t->root);
    verify_property_5(t->root);
#endif
}

void verify_property_1(node n) {
    assert(node_color(n) == RED || node_color(n) == BLACK);
    if (n == NULL) return;
    verify_property_1(n->left);
    verify_property_1(n->right);
}

void verify_property_2(node root) {
    assert(node_color(root) == BLACK);
}

color node_color(node n) {
    return n == NULL ? BLACK : n->color;
}

void verify_property_4(node n) {
    if (node_color(n) == RED) {
        assert (node_color(n->left)   == BLACK);
        assert (node_color(n->right)  == BLACK);
        assert (node_color(n->parent) == BLACK);
    }
    if (n == NULL) return;
    verify_property_4(n->left);
    verify_property_4(n->right);
}

void verify_property_5(node root) {
    int black_count_path = -1;
    verify_property_5_helper(root, 0, &black_count_path);
}

void verify_property_5_helper(node n, int black_count, int* path_black_count) {
    if (node_color(n) == BLACK) {
        black_count++;
    }
    if (n == NULL) {
        if (*path_black_count == -1) {
            *path_black_count = black_count;
        } else {
            assert (black_count == *path_black_count);
        }
        return;
    }
    verify_property_5_helper(n->left,  black_count, path_black_count);
    verify_property_5_helper(n->right, black_count, path_black_count);
}
/************************************** Operations **********************************/

rbtree rbtree_create() {
    rbtree t = (struct rbtree_t *)malloc(sizeof(struct rbtree_t));
    t->root = NULL;
    verify_properties(t);
 
    return t;
}

node new_node(void* key, void* value, color node_color, node left, node right) {
    node result = malloc(sizeof(struct rbtree_node_t));
    result->key = key;
    result->value = value;
    result->color = node_color;
    result->left = left;
    result->right = right;
    result->rwlock = malloc(sizeof(pthread_rwlock_t));
    /* Initialize the read-write lock */
    if(pthread_rwlock_init(result->rwlock, NULL)){
        printf("\n[ERROR] Could not create read-write lock");
        exit(1);
    }
    if (left  != NULL)  left->parent = result;
    if (right != NULL) right->parent = result;
    result->parent = NULL;
    return result;
}

node wrlock_branch(node n, int tid){
    int result=1;
    if(grandparent(grandparent(grandparent(grandparent(grandparent(n))))) != NULL){
#ifdef DEBUG
        printf("\n%u Thread: lock grandgrandgrandgrandparent, %p", tid, grandparent(grandparent(grandparent(grandparent(n)))));
	fflush(stdout);
#endif
	result = pthread_rwlock_wrlock(grandparent(grandparent(grandparent(grandparent(grandparent(n)))))->rwlock);
	if(result == EDEADLK){
	  printf("\n[ERROR] Current thread already owns the read-write lock");
	  exit(1);
	}
	else if(result == EINVAL){
	  printf("\n[ERROR]  value specified by rwlock does not refer to an initialised read-write lock");
	  exit(1);
	}
	return grandparent(grandparent(grandparent(grandparent(grandparent(n)))));
    }
    else if(grandparent(grandparent(grandparent(grandparent(n)))) != NULL){
#ifdef DEBUG
        printf("\n%u Thread: lock grandgrandgrandgrandparent, %p", tid, grandparent(grandparent(grandparent(grandparent(n)))));
	fflush(stdout);
#endif
	result = pthread_rwlock_wrlock(grandparent(grandparent(grandparent(grandparent(n))))->rwlock);
	if(result == EDEADLK){
	  printf("\n[ERROR] Current thread already owns the read-write lock");
	  exit(1);
	}
	else if(result == EINVAL){
	  printf("\n[ERROR]  value specified by rwlock does not refer to an initialised read-write lock");
	  exit(1);
	}
	return grandparent(grandparent(grandparent(grandparent(n))));
    }
    else if(grandparent(grandparent(grandparent(n))) != NULL){
#ifdef DEBUG
        printf("\n%u Thread: lock grandgrandgrandparent, %p", tid, grandparent(grandparent(grandparent(n))));
	fflush(stdout);
#endif
	result = pthread_rwlock_wrlock(grandparent(grandparent(grandparent(n)))->rwlock);
        if(result == EDEADLK){
	  printf("\n[ERROR] Current thread already owns the read-write lock");
	  exit(1);
	}
	else if(result == EINVAL){
	  printf("\n[ERROR]  value specified by rwlock does not refer to an initialised read-write lock");
	  exit(1);
	}
	return grandparent(grandparent(grandparent(n)));
    }
    else if(grandparent(grandparent(n)) != NULL){
#ifdef DEBUG
        printf("\n%u Thread: lock grandgrandparent, %p", tid, grandparent(grandparent(n)));
	fflush(stdout);
#endif
	result = pthread_rwlock_wrlock(grandparent(grandparent(n))->rwlock);
	if(result == EDEADLK){
	  printf("\n[ERROR] Current thread already owns the read-write lock");
	  exit(1);
	}
	else if(result == EINVAL){
	  printf("\n[ERROR]  value specified by rwlock does not refer to an initialised read-write lock");
	  exit(1);
	}
	return grandparent(grandparent(n));
    }
    else if(grandparent(n) != NULL){
#ifdef DEBUG
        printf("\n%u Thread: lock grandparent, %p", tid, grandparent(n));
        fflush(stdout);
#endif
	result = pthread_rwlock_wrlock(grandparent(n)->rwlock);
        if(result == EDEADLK){
	  printf("\n[ERROR] Current thread already owns the read-write lock");
	  exit(1);
	}
	else if(result == EINVAL){
	  printf("\n[ERROR]  value specified by rwlock does not refer to an initialised read-write lock");
	  exit(1);
	}
	return grandparent(n);
    }
    else if(n->parent != NULL){
#ifdef DEBUG
        printf("\n%u Thread: lock parent, %p", tid, n->parent);
        fflush(stdout);
#endif
	result = pthread_rwlock_wrlock(n->parent->rwlock);
        if(result == EDEADLK){
	  printf("\n[ERROR] Current thread already owns the read-write lock");
	  exit(1);
	}
	else if(result == EINVAL){
	  printf("\n[ERROR]  value specified by rwlock does not refer to an initialised read-write lock");
	  exit(1);
	}
	return n->parent;
    }
    else
        return NULL;
}

void unlock_branch(node lockednode, int tid){
    int result;
    if(lockednode != NULL){
#ifdef DEBUG
        printf("\n%u Thread: unlock node, %p", tid, lockednode);
	fflush(stdout);
#endif
        result = pthread_rwlock_unlock(lockednode->rwlock);
	if(result != 0){
	    printf("\n[ERROR] Couldn't unlock lock");
	    fflush(stdout);
	    exit(1);
	}
    }
}

node lookup_node(rbtree t, void* key, compare_func compare) {
    node n = t->root;
    while (n != NULL) {
        /* Shared lock */
        pthread_rwlock_rdlock(n->rwlock);
        int comp_result = compare(key, n->key);
        if (comp_result == 0) {
	    /* Unlock */
            pthread_rwlock_unlock(n->rwlock);
            return n;
        } else if (comp_result < 0) {
	    /* Unlock */
            pthread_rwlock_unlock(n->rwlock);
            n = n->left;
        } else {
            assert(comp_result > 0);
	    /* Unlock */
            pthread_rwlock_unlock(n->rwlock);
            n = n->right;
        }
    }
    return n;
}

void rotate_left(rbtree t, node n) {
    node r = n->right;
    replace_node(t, n, r);
    n->right = r->left;
    if (r->left != NULL) {
        r->left->parent = n;
    }
    r->left = n;
    n->parent = r;
}

void rotate_right(rbtree t, node n) {
    node L = n->left;
    replace_node(t, n, L);
    n->left = L->right;
    if (L->right != NULL) {
        L->right->parent = n;
    }
    L->right = n;
    n->parent = L;
}

void replace_node(rbtree t, node oldn, node newn) {
    if (oldn->parent == NULL) {
        t->root = newn;
    } else {
        if (oldn == oldn->parent->left)
            oldn->parent->left = newn;
        else
            oldn->parent->right = newn;
    }
    if (newn != NULL) {
        newn->parent = oldn->parent;
    }
}

int compare_int(void* leftp, void* rightp) {
    int left = (int)leftp;
    int right = (int)rightp;
    if (left < right) 
        return -1;
    else if (left > right)
        return 1;
    else {
        assert (left == right);
        return 0;
    }
}

void rbtree_insert(rbtree t, void* key, void* value, compare_func compare, int iID) {
    node inserted_node = new_node(key, value, RED, NULL, NULL);
    node lockednode = NULL;
    
    if (t->root == NULL) {
        t->root = inserted_node;
    } else {
        node n = t->root;
        while (1) {
	    lockednode = wrlock_branch(n, iID);
            int comp_result = compare(key, n->key);
            if (comp_result == 0) {
                n->value = value;
		unlock_branch(lockednode, iID);
                return;
            } else if (comp_result < 0) {
                if (n->left == NULL) {
                    n->left = inserted_node;
                    break;
                } else {
                    n = n->left;
                }
            } else {
                assert (comp_result > 0);
                if (n->right == NULL) {
                    n->right = inserted_node;
                    break;
                } else {
                    n = n->right;
                }
            }
            unlock_branch(lockednode, iID);
        }
        inserted_node->parent = n;
    }
#ifdef DEBUG
    printf("\nInserting Node");
    fflush(stdout);
#endif
    insert_case1(t, inserted_node);
    verify_properties(t);
    unlock_branch(lockednode, iID);
}

void rbtree_insert_ser(rbtree t, void* key, void* value, compare_func compare, int iID) {
    node inserted_node = new_node(key, value, RED, NULL, NULL);
    
    if (t->root == NULL) {
        t->root = inserted_node;
    } else {
        node n = t->root;
        while (1) {
            int comp_result = compare(key, n->key);
            if (comp_result == 0) {
                n->value = value;
                return;
            } else if (comp_result < 0) {
                if (n->left == NULL) {
                    n->left = inserted_node;
                    break;
                } else {
                    n = n->left;
                }
            } else {
                assert (comp_result > 0);
                if (n->right == NULL) {
                    n->right = inserted_node;
                    break;
                } else {
                    n = n->right;
                }
            }
        }
        inserted_node->parent = n;
    }
    insert_case1(t, inserted_node);
    verify_properties(t);
}

void insert_case1(rbtree t, node n) {
    if (n->parent == NULL)
        n->color = BLACK;
    else
        insert_case2(t, n);
}

void insert_case2(rbtree t, node n) {
    if (node_color(n->parent) == BLACK)
        return; /* Tree is still valid */
    else
        insert_case3(t, n);
}

void insert_case3(rbtree t, node n) {
    if (node_color(uncle(n)) == RED) {
        n->parent->color = BLACK;
        uncle(n)->color = BLACK;
        grandparent(n)->color = RED;
        insert_case1(t, grandparent(n));
    } else {
        insert_case4(t, n);
    }
}

void insert_case4(rbtree t, node n) {
    if (n == n->parent->right && n->parent == grandparent(n)->left) {
        rotate_left(t, n->parent);
        n = n->left;
    } else if (n == n->parent->left && n->parent == grandparent(n)->right) {
        rotate_right(t, n->parent);
        n = n->right;
    }
    insert_case5(t, n);
}

void insert_case5(rbtree t, node n) {
    n->parent->color = BLACK;
    grandparent(n)->color = RED;
    if (n == n->parent->left && n->parent == grandparent(n)->left) {
        rotate_right(t, grandparent(n));
    } else {
        assert (n == n->parent->right && n->parent == grandparent(n)->right);
        rotate_left(t, grandparent(n));
    }
}

void print_tree(rbtree t) {
    print_tree_helper(t->root, 0);
    puts("");
}

void print_tree_helper(rbtree_node n, int indent) {
    int i;
    if (n == NULL) {
        fputs("<empty tree>", stdout);
        return;
    }
    if (n->right != NULL) {
        print_tree_helper(n->right, indent + INDENT_STEP);
    }
    for(i=0; i<indent; i++)
        fputs(" ", stdout);
    if (n->color == BLACK)
        printf("%d\n", (int)n->key);
    else
        printf("<%d>\n", (int)n->key);
    if (n->left != NULL) {
        print_tree_helper(n->left, indent + INDENT_STEP);
    }
}

void* rbtree_lookup(rbtree t, void* key, compare_func compare) {
    /* Shared lock */
    //pthread_rwlock_rdlock(&rwlock);
    node n = lookup_node(t, key, compare);
    /* Unlock */
    //pthread_rwlock_unlock(&rwlock);
    return n == NULL ? NULL : n->value;
}