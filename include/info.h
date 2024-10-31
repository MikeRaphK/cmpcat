#ifndef INFO_H
#define INFO_H

#include "avltree.h"    // AVLTree

// Global info will be shared among the source files through a variable called 'info'
typedef struct {
    char *hierarchyA;           // Absolute path of hierarchyA
    char *hierarchyB;           // Absolute path of hierarchyB
    char *hierarchyC;           // Absolute path of hierarchyC
    char *exeDir;               // Absolute path of the executable

    size_t lenA;                // Results of strlen for the above 
    size_t lenB;                // paths so we don't call strlen 
    size_t lenC;                // multiple times
    size_t lenExe;

    AVLTree *avl_hardlinks;     // This AVL tree will be used to manage hardlinks
} GlobalInfo;

// Initialize the global variable 'info'
void info_init(int argc, char *pathA, char *pathB, char *pathC);

// Destroy the global variable 'info'
void info_destroy(int argc);

#endif