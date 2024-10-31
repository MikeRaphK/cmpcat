#ifndef CAT_MANAGER_H
#define CAT_MANAGER_H 

#include "wrapper.h"    // ArrayWrapper

// Find and print the differences between 2 catalogs
void find_differences(ArrayWrapper *wrapperA, ArrayWrapper *wrapperB);

// Find and print the differences between 2 catalogs. Also merge them in a new catalog
void find_and_merge(ArrayWrapper *wrapperA, ArrayWrapper *wrapperB);

#endif