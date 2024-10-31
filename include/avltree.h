#ifndef AVLTREE_H
#define AVLTREE_H

#include <sys/types.h>  // ino_t

typedef struct avl_tree AVLTree;

// Initializes and returns an empty AVL tree
AVLTree *avl_create(void);

// Inserts given iNode-Path pair in given AVL tree. Duplicates are not allowed
void avl_insert(AVLTree *avl, ino_t inode, char *path);

// Searches for given iNode in the AVL tree.
// If found, returns the corresponding path, otherwise NULL
char *avl_find(AVLTree *avl, ino_t inode);

// Destroys given AVL tree
void avl_destroy(AVLTree *avl);

#endif