#include <dirent.h>         // DIR etc.
#include <stdio.h>          // perror() etc.
#include <stdlib.h>         // EXIT_FAILURE
#include <string.h>         // strlen() etc.

#include "cat_manager.h"
#include "info.h"           // GlobalInfo

extern GlobalInfo *info;

// Find and print the differences between two catalogs
void find_differences(ArrayWrapper *wrapperA, ArrayWrapper *wrapperB) {
    // Look for differences only in the common hierarchy-levels of the catalogs, since the 
    // uncommmon ones (one catalog has more levels than the other) are unique
    int commonLevels = (wrapperA->lastLevel < wrapperB->lastLevel) ? wrapperA->lastLevel : wrapperB->lastLevel;
    for (int level = 0; level <= commonLevels; level++) {
        // Traverse the entries of hierarchyA in current level and compare them to the ones of hierarchyB
        for (int i = wrapperA->levels[level]; i < wrapperA->levels[level+1]; i++) {
            // Flag to check if the current entry is the same with another entry in hierarchyB
            int same = 0;
            for (int j = wrapperB->levels[level]; j < wrapperB->levels[level+1]; j++) {
                // If entries have either different types OR names, continue
                if (wrapperA->array[i]->fileType != wrapperB->array[j]->fileType) continue;
                if (strcmp(wrapperA->array[i]->relativeToHier, wrapperB->array[j]->relativeToHier)) continue;

                same = entries_are_same(wrapperA->array[i], wrapperB->array[j]);

                // No need to keep searching further
                break;
            }
            // If no other same entry was found, print the difference
            if (!same) printf("\t%s\n", wrapperA->array[i]->relativePath);
        }
    }
    // Mark each entry of each extra-level of hierarchyA as a difference
    for (int level = commonLevels + 1; level <= wrapperA->lastLevel; level++) {
        for (int i = wrapperA->levels[level]; i < wrapperA->levels[level+1]; i++) {
            printf("\t%s\n", wrapperA->array[i]->relativePath);
        }
    }
}

// Find and print the differences between two catalogs. Also merge them in a new catalog
void find_and_merge(ArrayWrapper *wrapperA, ArrayWrapper *wrapperB) {
    // Open dirC
    DIR *dirC = opendir(info->hierarchyC);
    if (dirC == NULL) {
        perror("opendir()");
        exit(EXIT_FAILURE);
    }
    // Look for differences only in the common hierarchy-levels of the catalogs, since the 
    // uncommmon ones (one catalog has more levels than the other) are unique and should be merged
    int commonLevels = (wrapperA->lastLevel < wrapperB->lastLevel) ? wrapperA->lastLevel : wrapperB->lastLevel;
    for (int level = 0; level <= commonLevels; level++) {
        // Traverse the entries of hierarchyA in current level and compare them to the ones of hierarchyB
        for (int i = wrapperA->levels[level]; i < wrapperA->levels[level+1]; i++) {
            // Flags to check if entryA was found/merged or if another entry was already created
            int same = 0, merged = 0, created = 0;
            for (int j = wrapperB->levels[level]; j < wrapperB->levels[level+1]; j++) {
                // If the 2 entries have a different name, ignore them
                if (strcmp(wrapperA->array[i]->relativeToHier, wrapperB->array[j]->relativeToHier)) continue;

                // If 2 entries have the same name and B's entry is newer OR A and B have the
                // same modified time, keep B. Update the flag so A won't also get merged
                if (wrapperA->array[i]->mtime <= wrapperB->array[j]->mtime) {
                    create_entry(wrapperB->array[j]);
                    created = 1;
                }
                // Else if A was a newer directory, create it so you can also create future children
                else if (wrapperA->array[i]->fileType == DIRECTORY) {
                    create_entry(wrapperA->array[i]);
                    created = 1;
                }
                merged = 1;

                // If the 2 entries have different types, then there is no need to compare them
                if (wrapperA->array[i]->fileType == wrapperB->array[j]->fileType) {
                    // Here, the 2 entries have the same name AND file type. Compare them
                    same = entries_are_same(wrapperA->array[i], wrapperB->array[j]);
                }
                break;
            }
            // If no other same entry was found, print the difference
            if (!same) printf("\t%s\n", wrapperA->array[i]->relativePath);
            // If no other same entry was found and B's entry was not created, create entryA in dirC
            if (!merged && !created) create_entry(wrapperA->array[i]);
        }
    }
    // Mark each entry of each extra-level of hierarchyA as a difference and add it to dirC
    for (int level = commonLevels + 1; level <= wrapperA->lastLevel; level++) {
        for (int i = wrapperA->levels[level]; i < wrapperA->levels[level+1]; i++) {
            printf("\t%s\n", wrapperA->array[i]->relativePath);
            create_entry(wrapperA->array[i]);
        }
    }
    if (closedir(dirC) == -1) {
        perror("closedir()");
        exit(EXIT_FAILURE);
    }
}