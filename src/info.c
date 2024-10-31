#include <stdio.h>      // perror()
#include <stdlib.h>     // free() etc.
#include <string.h>     // strlen()

#include "info.h"
#include "utils.h"      // NULL_CHECK()

extern GlobalInfo *info;

// Initialize the global variable 'info'
void info_init(int argc, char *pathA, char *pathB, char *pathC) {
    info = malloc(sizeof(*info));
    NULL_CHECK(info, "malloc");

    // Get realpath and length of pathA
    char *temp = realpath(pathA, NULL);
    NULL_CHECK(temp, "realpath");
    info->hierarchyA = fix_path(temp);
    free(temp);
    info->lenA = strlen(info->hierarchyA);

    // Get realpath and length of pathB
    temp = realpath(pathB, NULL);
    NULL_CHECK(temp, "realpath");
    info->hierarchyB = fix_path(temp);
    free(temp);
    info->lenB = strlen(info->hierarchyB);

    // Get realpath and length of exeDir
    temp = realpath(".", NULL);
    NULL_CHECK(temp, "realpath");
    info->exeDir = fix_path(temp);
    free(temp);
    info->lenExe = strlen(info->exeDir);

    // If user wants to also merge, get realpath and length of pathC
    if (argc == 6) {
        temp = realpath(pathC, NULL);
        NULL_CHECK(temp, "realpath");
        info->hierarchyC = fix_path(temp);
        free(temp);
        info->lenC = strlen(info->hierarchyC);
        info->avl_hardlinks = avl_create();
    }
    else {
        info->hierarchyC = NULL;
        info->lenC = 0;
        info->avl_hardlinks = NULL;
    }
}

// Destroy the global variable 'info'
void info_destroy(int argc) {
    free(info->hierarchyA);
    free(info->hierarchyB);
    free(info->exeDir);
    if (argc == 6) {
        free(info->hierarchyC);
        avl_destroy(info->avl_hardlinks);
    }
    free(info);
}