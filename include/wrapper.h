#ifndef WRAPPER_H
#define WRAPPER_H

#include "entry_manager.h"  // EntryInfo

// Wrapper used to save information about the array of entries
typedef struct {
    int index;          // Current index of array
    int size;           // Total size of array
    EntryInfo **array;  // The array of EntryInfo pointers
    int *levels;        // Array in which position i refers to the start of level-i in above array
    int lastLevel;      // Last level of array
    char fromHierarchy; // Indicates from which hierarchy this array comes from. Uses #defines listed above
} ArrayWrapper;

// Initialize a wrapper
ArrayWrapper *wrapper_init(char *path, char fromHierarchy);

// Destroy a wrapper using appropriate memory deallocation
void wrapper_destroy(ArrayWrapper *wrapper);

#endif