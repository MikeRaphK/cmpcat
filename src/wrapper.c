#include <dirent.h>     // DIR etc
#include <stdio.h>      // perror() etc.
#include <stdlib.h>     // malloc() etc.
#include <string.h>     // strcpy() etc.

#include "info.h"       // GlobalInfo
#include "utils.h"      // NULL_CHECK() etc.
#include "wrapper.h"

extern GlobalInfo *info;

// Initialize an array of EntryInfo pointers
static void array_init(char *path, ArrayWrapper *wp) {
    DIR *dir;
    struct dirent *entry;

    // GOAL: Store hierarchy's entries per level in the array, 
    // so as to ease and optimize the traversals of it later

    // Dynamic 1D-string-array buffer that stores the directories of the current level
    int currCapacity = 64;
    char **currDirs = malloc(currCapacity * sizeof(*currDirs));
    NULL_CHECK(currDirs, "malloc");
    int currIndex = 0;
    // Dynamic 1D-string-array buffer that stores the directories of the next (new) level
    int newCapacity = 64;
    char **newDirs = malloc(newCapacity * sizeof(*newDirs));
    NULL_CHECK(newDirs, "malloc");
    int newIndex = 0;

    // At first, the only current directory is the one given
    currDirs[currIndex] = path;
    currIndex++;

    int levelsCounter = 0;

    // For every level of the hierarchy
    while (1) {
        if (levelsCounter == wp->lastLevel) {
            wp->lastLevel *= 2;
            wp->levels = realloc(wp->levels, wp->lastLevel * sizeof(*wp->levels));
            NULL_CHECK(wp->levels, "realloc");
        }
        wp->levels[levelsCounter] = wp->index; // Index of where each level starts in the array
        levelsCounter++;

        // For every directory of current level
        while (currIndex--) {
            if ((dir = opendir(currDirs[currIndex])) == NULL) {
                perror("opendir()");
                exit(EXIT_FAILURE);
            }

            // For every entry of current directory of current level
            while ((entry = readdir(dir)) != NULL) {
                // Ignore parent and current folder
                if (!strcmp(entry->d_name, "..") || !strcmp(entry->d_name, ".")) continue;

                // Get relative path
                char *relative = concatinate(currDirs[currIndex], entry->d_name);

                // Initialize an entry inside the array
                if (wp->index == wp->size) {
                    wp->size *= 2;
                    wp->array = realloc(wp->array, wp->size * sizeof(EntryInfo *));
                    NULL_CHECK(wp->array, "realloc");
                }
                // NULL is only returned when faced with a symlink that points outside its hierarchy
                if ((wp->array[wp->index] = entry_init(relative, entry->d_name, wp->fromHierarchy)) != NULL) {
                    if (wp->array[wp->index]->fileType == DIRECTORY) {
                        // Mark subdirectory to expand/traverse in the next level
                        if (newIndex == newCapacity) {
                            newCapacity *= 2;
                            newDirs = realloc(newDirs, newCapacity * sizeof(*newDirs));
                            NULL_CHECK(newDirs, "realloc");
                        }
                        newDirs[newIndex] = wp->array[wp->index]->relativePath;
                        newIndex++;
                    }
                    wp->index++;
                }
                free(relative);
            }

            if (closedir(dir) == -1) {
                perror("closedir()");
                exit(EXIT_FAILURE);
            }
        }

        // No subdirectories; No more levels; Exit the loop
        if (newIndex == 0) break;

        // Move on to the next hierarchy-level
        if (currCapacity < newIndex) {
            // Double capacity until newDirs contents fit
            do { currCapacity *= 2; } while (currCapacity < newIndex);
            currDirs = realloc(currDirs, currCapacity * sizeof(*currDirs));
            NULL_CHECK(currDirs, "realloc");
        }
        // For the new level, currDirs is the previous's level newDirs
        memcpy(currDirs, newDirs, newIndex * sizeof(*newDirs));
        currIndex = newIndex;

        // Reset newDirs
        newIndex = 0;
    }

    // Store where the last level ends
    wp->levels[levelsCounter] = wp->index;
    // Position of the start of the last level
    wp->lastLevel = levelsCounter - 1;

    free(currDirs);
    free(newDirs);
}

// Initialize a wrapper
ArrayWrapper *wrapper_init(char *path, char fromHierarchy) {
    ArrayWrapper *wrapper = malloc(sizeof(*wrapper));
    NULL_CHECK(wrapper, "malloc");

    wrapper->fromHierarchy = fromHierarchy;

    wrapper->levels = malloc(8 * sizeof(*wrapper->levels));
    NULL_CHECK(wrapper->levels, "malloc");
    wrapper->lastLevel = 7;

    // Wrapper is initially empty with a temporary max size of 8
    // If the size of the array is exceeded, it then gets doubled
    wrapper->index = 0;
    wrapper->size = 8;
    wrapper->array = malloc(wrapper->size * sizeof(EntryInfo *));
    NULL_CHECK(wrapper->array, "malloc");
    array_init(path, wrapper);
    // After inserting all entries, get the array's max size
    wrapper->size = wrapper->index;

    return wrapper;
}

// Destroy a wrapper using appropriate memory deallocation
void wrapper_destroy(ArrayWrapper *wrapper) {
    for (int i = 0; i < wrapper->size; i++) {
        entry_destroy(wrapper->array[i]);
    }
    free(wrapper->array);
    free(wrapper->levels);
    free(wrapper);
}
