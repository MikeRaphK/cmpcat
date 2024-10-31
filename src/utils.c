#include <errno.h>      // errno
#include <stdio.h>      // perror()
#include <stdlib.h>     // malloc() etc.
#include <string.h>     // strcpy() etc.
#include <unistd.h>     // read() etc.

#include "utils.h"

// A modified version of the `read` syscall that reads all the desired bytes
void fullread(int fd, void *buf, size_t count) {
    if (count == 0) {
        return;
    }
    ssize_t cbr;    // current number of bytes read
    size_t tbr = 0; // total number of bytes read
    char *bufc = buf; // convert to (char *) to allow pointer arithmetic
    while (tbr < count) {
        cbr = read(fd, bufc + tbr, count - tbr);
        if (cbr == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("read()");
            exit(EXIT_FAILURE);
        }
        tbr += cbr;
    }
}

// A modified version of the `write` syscall that writes all the desired bytes
void fullwrite(int fd, void *buf, size_t count) {
    if (count == 0) {
        return;
    }
    ssize_t cbw;    // current number of bytes written
    size_t tbw = 0; // total number of bytes written
    char *bufc = buf; // convert to (char *) to allow pointer arithmetic
    while (tbw < count) {
        cbw = write(fd, bufc + tbw, count - tbw);
        if (cbw == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("write()");
            exit(EXIT_FAILURE);
        }
        tbw += cbw;
    }
}

// Return a copy of a given string
char *duplicate_string(char *string) {
    char *new = malloc((strlen(string)+1) * sizeof(char));
    NULL_CHECK(new, "malloc");
    strcpy(new, string);
    return new;
}

// Return the given string reversed
char *reverse_of(char *string) {
    size_t length = strlen(string);
    char *reversed = malloc((length+1) * sizeof(char));
    NULL_CHECK(reversed, "malloc");
    for (size_t i = 0, j = length-1 ; i < length ; i++, j--) {
        reversed[i] = string[j];
    }
    reversed[length] = '\0';
    return reversed;
}

// Add '/' between a path and an entry name
char *concatinate(char *currentPath, char *entryName) {
    char *totalPath;
    size_t currentLen = strlen(currentPath);
    if (currentPath[currentLen-1] == '/') {
        totalPath = malloc((currentLen+strlen(entryName)+1) * sizeof(char));
        NULL_CHECK(totalPath, "malloc");
        strcpy(totalPath, currentPath);
        strcat(totalPath, entryName);
    }
    else {
        totalPath = malloc((currentLen+strlen(entryName)+2) * sizeof(char));
        NULL_CHECK(totalPath, "malloc");
        strcpy(totalPath, currentPath);
        strcat(totalPath, "/");
        strcat(totalPath, entryName);
    }
    return totalPath;
}

// Return the absolute path of a given path
char *get_absolute_path(char *from, char *relative) {
    // Append 'relative' to 'from' to get 'total'. This way
    // we can start searching from the end and remove all
    // '..' and '.' ocurrences
    size_t totalLen = strlen(from)+strlen(relative);
    char *total = malloc((totalLen+1)*sizeof(char));
    NULL_CHECK(total, "malloc");
    strcpy(total, from);
    strcat(total, relative);

    // Reverse total in order to start searching from the end
    char *reversed = reverse_of(total);
    size_t revLen = totalLen;

    // revAbs will be 'reversed' without '..' or '.'
    size_t capacity = 256; // Initial buffer capacity
    char *revAbs = malloc(capacity * sizeof(char));
    NULL_CHECK(revAbs, "malloc");
    size_t revAbsIndex = 0;
    
    // For every character of reversed
    for (size_t i = 0 ; i < revLen ; i++) {
        // Counter to check how many tokens we need to ignore. Tokens are marked by '/'
        int ignore = 0;
        // Start counting how many '..' exist in a row
        while (reversed[i] == '.' && reversed[i+1] == '.') {
            i += 3;
            ignore++;
        }
        // If you encountered '..'
        if (ignore) {
            // Skip tokens 
            while (ignore) {
                if (reversed[i] == '/') ignore--;
                if (reversed[i] == '.') ignore++;   // If a './' occurs, ignore some more
                i++;
            }
            // Do i-- since i will be incremented by the for loop
            i--;
        }
        // Else if you encountered "./" just ignore it
        else if (reversed[i] == '.' && reversed[i+1] == '/') i++;
        // Else if it a normal character, copy it
        else {
            // If buffer is full, double its size
            if (revAbsIndex == capacity - 1) {
                capacity *= 2;
                revAbs = realloc(revAbs, capacity * sizeof(char));
                NULL_CHECK(revAbs, "realloc");
            }
            revAbs[revAbsIndex] = reversed[i];
            revAbsIndex++;
        }
    }
    // Don't forget to add '\0' at the end 
    revAbs[revAbsIndex] = '\0';

    free(total);
    free(reversed);

    char *result = reverse_of(revAbs);

    free(revAbs);

    return result;
}

// Paths start with ./ and end with /
char *fix_path(char *path) {
    char *fixed;
    size_t pathLen = strlen(path);

    // Check if the path starts with '~'
    if (path[0] == '~') {
        char *homeDir = getenv("HOME");
        size_t len = strlen(homeDir);
        // Replace '~' with '/home/user'
        memmove(path + len, path + 1, strlen(path));
        memcpy(path, homeDir, len);
    }
    // Case: Path is absolute and ends with '/'
    if (path[0] == '/' && path[pathLen-1] == '/') fixed = duplicate_string(path);
    // Case: Path is absolute and doesn't end with '/'
    else if (path[0] == '/') {
        fixed = malloc((pathLen+2) * sizeof(char));
        NULL_CHECK(fixed, "malloc");
        strcpy(fixed, path);
        strcat(fixed, "/");
    }
    // Case: Path starts with './' and ends with '/'
    else if (path[0] == '.' && path[1] == '/' && path[pathLen-1] == '/') fixed = duplicate_string(path);
    // Case: Path starts with './' and doesn't end with '/'
    else if (path[0] == '.' && path[1] == '/') {
        fixed = malloc((pathLen+2) * sizeof(char));
        NULL_CHECK(fixed, "malloc");
        strcpy(fixed, path);
        strcat(fixed, "/");
    }
    // Case: Path doesn't start with './' and ends with '/'
    else if (path[pathLen-1] == '/') {
        fixed = malloc((pathLen+3) * sizeof(char));
        NULL_CHECK(fixed, "malloc");
        fixed[0] = '.';
        fixed[1] = '/';
        fixed[2] = '\0';
        strcat(fixed, path);
    }
    // Case: Path doesn't start with './' and doesn't end with '/'
    else {
        fixed = malloc((pathLen+4) * sizeof(char));
        NULL_CHECK(fixed, "malloc");
        fixed[0] = '.';
        fixed[1] = '/';
        fixed[2] = '\0';
        strcat(fixed, path);
        strcat(fixed, "/");
    }
    return fixed;
}