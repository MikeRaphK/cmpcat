#include <stdio.h>      // fprintf() etc.
#include <stdlib.h>     // malloc() etc.

#include "avltree.h"
#include "utils.h"      // NULL_CHECK()

typedef struct avl_node AVLNode;
struct avl_node {
    ino_t inode;
    char *path;
    AVLNode *left;
    AVLNode *right;
    int height; // Height of the tree in the current node
};

struct avl_tree {
    AVLNode *root;
};

AVLTree *avl_create(void) {
    AVLTree *avl = calloc(1, sizeof(*avl));
    NULL_CHECK(avl, "calloc");
    return avl;
}

// A function to create a new node.
// Height of node is initially 1, since the simple bst insertion inserts the node as a leaf
static AVLNode *avl_node_create(ino_t inode, char *path) {
    AVLNode *node = malloc(sizeof(*node));
    NULL_CHECK(node, "malloc");
    node->inode = inode;
    node->path = duplicate_string(path);
    node->left = NULL;
    node->right = NULL;
    node->height = 1;
    return node;
}

// Return the height of a node
static int avl_height(AVLNode *node) {
    return (node != NULL) ? node->height : 0;
}

// Return the balance of a current node
static int avl_balance(AVLNode *node) {
    return (node != NULL) ? avl_height(node->left) - avl_height(node->right) : 0;
}

static int max_int(int x, int y) {
    return (x > y) ? x : y;
}

// Function to perform a right rotation on a node
static AVLNode *avl_right_rotate(AVLNode *y) {
    AVLNode *x = y->left;
    if (x == NULL) return y;
    AVLNode *T2 = x->right;

    x->right = y;
    y->left = T2;

    // update heights
    y->height = max_int(avl_height(y->left), avl_height(y->right)) + 1;
    x->height = max_int(avl_height(x->left), avl_height(x->right)) + 1;

    return x;
}

// Function to perform a left rotation on a node
static AVLNode *avl_left_rotate(AVLNode *x) {
    AVLNode *y = x->right;
    if (y == NULL) return x;
    AVLNode *T2 = y->left;

    y->left = x;
    x->right = T2;

    // update heights
    x->height = max_int(avl_height(x->left), avl_height(x->right)) + 1;
    y->height = max_int(avl_height(y->left), avl_height(y->right)) + 1;

    return y;
}

// Function to insert a node in an AVL tree
static AVLNode *avl_node_insert(AVLNode *node, ino_t inode, char *path) {
    // 1. Perform a simple bst tree insertion
    if (node == NULL) return avl_node_create(inode, path);
    else if (inode > node->inode) node->right = avl_node_insert(node->right, inode, path);
    else if (inode < node->inode) node->left = avl_node_insert(node->left, inode, path);
    else {
        fprintf(stderr, "No duplicates in AVL tree\n");
        exit(EXIT_FAILURE);
    }

    // 2. update the height of the node
    node->height = max_int(avl_height(node->right), avl_height(node->left)) + 1;

    // 3. get the balance factor of the current node
    int balance = avl_balance(node);

    // 4. perform the rotations

    if (balance > 1 && inode < node->left->inode) {
        return avl_right_rotate(node);
    }
    else if (balance > 1 && inode > node->left->inode) {
        node->left = avl_left_rotate(node->left);
        return avl_right_rotate(node);
    }
    else if (balance < -1 && inode > node->right->inode) {
        return avl_left_rotate(node);
    }
    else if (balance < -1 && inode < node->right->inode) {
        node->right = avl_right_rotate(node->right);
        return avl_left_rotate(node);
    }
    return node;
}

void avl_insert(AVLTree *avl, ino_t inode, char *path) {
    avl->root = avl_node_insert(avl->root, inode, path);
}

char *avl_find(AVLTree *avl, ino_t inode) {
    AVLNode *node = avl->root;
    while (node != NULL) {
        if (inode == node->inode) return node->path;
        else if (inode < node->inode) node = node->left;
        else node = node->right;
    }
    return NULL;
}

static void avl_node_destroy(AVLNode *node) {
    if (node != NULL) {
        avl_node_destroy(node->left);
        avl_node_destroy(node->right);
        free(node->path);
        free(node);
    }
}

void avl_destroy(AVLTree *avl) {
    avl_node_destroy(avl->root);
    free(avl);
}