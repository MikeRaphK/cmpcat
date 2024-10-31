#ifndef UTILS_H
#define UTILS_H

#define BUFLEN 4096

#define NULL_CHECK(cond, funcName)      \
    if ((cond) == NULL) {               \
        perror(funcName "()");          \
        exit(EXIT_FAILURE);             \
    }

// A modified version of the `read` syscall that reads all the desired bytes
void fullread(int fd, void *buf, size_t count);

// A modified version of the `write` syscall that writes all the desired bytes
void fullwrite(int fd, void *buf, size_t count);

// Return a copy of a given string
char *duplicate_string(char *string);

// Return the given string reversed
char *reverse_of(char *string);

// Add '/' between a path and an entry name
char *concatinate(char *currentPath, char *entryName);

// Return the absolute path of a given path
char *get_absolute_path(char *from, char *relative);

// Paths start with ./ and end with /
char *fix_path(char *path);

#endif