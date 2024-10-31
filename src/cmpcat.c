#include <dirent.h>         // DIR etc.
#include <stdio.h>          // printf() etc.
#include <stdlib.h>         // EXIT_FAILURE
#include <string.h>         // strlen() etc.
#include <sys/stat.h>       // mkdir()

#include "cat_manager.h"    // find_differences() etc.
#include "info.h"           // GlobalInfo
#include "utils.h"          // fix_path() etc.
#include "wrapper.h"        // ArrayWrapper

GlobalInfo *info; // Global info will be shared among the source files

// Helper function to correctly parse given arguements
static void parse_args(int argc, char *argv[], char **pathA, char **pathB, char **pathC) {
    // User can either run the program to only compare OR compare and merge
    if (argc != 6 && argc != 4) {
        fprintf(stderr, "Usage: %s -d <pathA> <pathB> OR %s -d <pathA> <pathB> -s <pathC>\n", argv[0], argv[0]);
        exit(EXIT_FAILURE);
    }

    // Case: User only wants to compare dirs
    if (argc == 4) {
        if (strcmp(argv[1], "-d")) {
            fprintf(stderr, "Usage: %s -d <pathA> <pathB> OR %s -d <pathA> <pathB> -s <pathC>\n", argv[0], argv[0]);
            exit(EXIT_FAILURE);
        }
        *pathA = fix_path(argv[2]);
        *pathB = fix_path(argv[3]);
        *pathC = NULL;  // For good measure
        return;
    }

    // Case: User wants to compare AND merge dirs together
    if (!strcmp(argv[1], "-d")) {
        *pathA = fix_path(argv[2]);
        *pathB = fix_path(argv[3]);
        if (strcmp(argv[4], "-s")) {
            fprintf(stderr, "Usage: %s -d <pathA> <pathB> OR %s -d <pathA> <pathB> -s <pathC>\n", argv[0], argv[0]);
            exit(EXIT_FAILURE);
        }
        *pathC = fix_path(argv[5]);
    }
    else if (!strcmp(argv[1], "-s")) {
        *pathC = fix_path(argv[2]);
        if (strcmp(argv[3], "-d")) {
            fprintf(stderr, "Usage: %s -d <pathA> <pathB> OR %s -d <pathA> <pathB> -s <pathC>\n", argv[0], argv[0]);
            exit(EXIT_FAILURE);
        }
        *pathA = fix_path(argv[4]);
        *pathB = fix_path(argv[5]);
    }
    else {
        fprintf(stderr, "Usage: %s -d <pathA> <pathB> OR %s -d <pathA> <pathB> -s <pathC>\n", argv[0], argv[0]);
        exit(EXIT_FAILURE);
    }

    // If dirC exists, make sure it is empty
    DIR *dirC;
    struct dirent *dirEntryC;
    int n = 0;
    if ((dirC = opendir(*pathC)) != NULL) {
        // Read all entries. Entries should only be current and parent folder
        while ((dirEntryC = readdir(dirC)) != NULL) {
            if (++n > 2) {
                fprintf(stderr, "%s is not empty\n", *pathC);
                exit(EXIT_FAILURE);
            }
        }
        if (closedir(dirC) == -1) {
            perror("closedir()");
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[]) {
    char *pathA, *pathB, *pathC;
    parse_args(argc, argv, &pathA, &pathB, &pathC);

    DIR *dirA, *dirB, *dirC; 
    // Check if directories exist
    if ((dirA = opendir(pathA)) == NULL) {
        perror("opendir()");
        exit(EXIT_FAILURE);
    }
    if ((dirB = opendir(pathB)) == NULL) {
        perror("opendir()");
        exit(EXIT_FAILURE);
    }
    if (closedir(dirA) == -1 || closedir(dirB) == -1) {
        perror("closedir()");
        exit(EXIT_FAILURE);
    }
    // If user wants to merge, create dirC if it doesn't already exist
    if (argc == 6 && (dirC = opendir(pathC)) == NULL) {
        mkdir(pathC, 0755);
        dirC = opendir(pathC);
        if (closedir(dirC) == -1) {
            perror("closedir()");
            exit(EXIT_FAILURE);
        }
    }

    // Initialize global info
    info_init(argc, pathA, pathB, pathC);

    // Initialize wrapperA
    ArrayWrapper *wrapperA = wrapper_init(pathA, HIER_A);

    // Initialize wrapperB
    ArrayWrapper *wrapperB = wrapper_init(pathB, HIER_B);

    // Case: User only wants to find differences
    if (argc == 4) {
        printf("In pathA :\n");
        find_differences(wrapperA, wrapperB);
        printf("In pathB :\n");
        find_differences(wrapperB, wrapperA);
    }
    // Case: User want to find differences and merge the dirs
    else {
        printf("In pathA :\n");
        find_and_merge(wrapperA, wrapperB);
        printf("In pathB :\n");
        find_and_merge(wrapperB, wrapperA);
    }
    
    // Destroy global info
    info_destroy(argc);

    // Destroy the wrappers
    wrapper_destroy(wrapperA);
    wrapper_destroy(wrapperB);

    free(pathA);
    free(pathB);
    if (argc == 6) free(pathC);
    
    return 0;
}