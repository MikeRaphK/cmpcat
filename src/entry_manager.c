#include <errno.h>              // errno
#include <fcntl.h>              // O_FLAGS
#include <stdio.h>              // fprintf() etc.
#include <stdlib.h>             // exit() etc.
#include <string.h>             // strlen() etc.
#include <sys/stat.h>           // mkdir() etc.
#include <unistd.h>             // link() etc.

#include "entry_manager.h"
#include "info.h"               // GlobalInfo
#include "utils.h"              // NULL_CHECK() etc.

extern GlobalInfo *info;

// Initialize an entry
EntryInfo *entry_init(char *relativePath, char *name, char fromHierarchy) {
    // Stat init
    struct stat myStat;
    if (lstat(relativePath, &myStat) == -1) {
        perror("lstat()");
        exit(EXIT_FAILURE);
    }

    // File type init
    char fileType;
    switch (myStat.st_mode & __S_IFMT) {
        case __S_IFREG:
            if (myStat.st_nlink > 1) fileType = HARDLINK;
            else fileType = REGFILE;
            break;
        case __S_IFDIR:
            fileType = DIRECTORY;
            break;
        case __S_IFLNK:
            // Ignore symlinks pointing outside the hierarchy
            if (!symlink_in_hierachy(relativePath, fromHierarchy)) return NULL;
            fileType = SYMLINK;
            break;
        default:
            fprintf(stderr, "Unkown file type\n");
            exit(EXIT_FAILURE);
    }

    // We allocate memory here in case the previous switch returns NULL
    EntryInfo *entry = malloc(sizeof(*entry));
    NULL_CHECK(entry, "malloc");
    // File Type
    entry->fileType = fileType;
    // Inode
    entry->inode = myStat.st_ino;
    // Size
    entry->size = myStat.st_size;
    // Mtime
    entry->mtime = myStat.st_mtime;
    // Permissions
    entry->perms = myStat.st_mode;

    // Relative path
    // Case: relativePath is a relative path
    if (relativePath[0] != '/') entry->relativePath = duplicate_string(relativePath);
    // Case: relativePath is an absolute path. Convert it to relative before continuing
    else {
        // Case: Executable's hierarchy is a substring of relativePath's absolute path
        if (strstr(relativePath, info->exeDir) != NULL) {
            entry->relativePath = malloc((strlen(relativePath) - info->lenExe + 3) * sizeof(char));
            NULL_CHECK(entry->relativePath, "malloc");
            entry->relativePath[0] = '.';
            entry->relativePath[1] = '\0';
            strcat(entry->relativePath, relativePath+info->lenExe-1);
        }
        // Case: Executable's hierarchy is deeper than relativePath's absolute path
        else {
            // Get the offset of the folders that are common in both hierarchies
            int i = 0, offset = 0, parentsCount = 0;
            while (info->exeDir[i] == relativePath[i]) {
                if (info->exeDir[i] == '/') offset = i+1;
                i++;
            }

            // Count how many folders of executable's path need to be replaced by '..'
            char *duplicate = duplicate_string(info->exeDir+offset);
            char *token = strtok(duplicate, "/");
            while (token != NULL) {
                parentsCount++;
                token = strtok(NULL, "/");
            }

            // Since executable is deeper than relativePath, build entry->relativePath using '..'
            size_t len = strlen(relativePath+offset) + 3*parentsCount;
            entry->relativePath = malloc((len + 1) * sizeof(char));
            i = 0;
            while (parentsCount) {
                strcpy(entry->relativePath+i, "../");
                i += 3;
                parentsCount--;
            }
            strcat(entry->relativePath, relativePath+offset);
            entry->relativePath[len] = '\0';

            free(duplicate);
        }
    }
    
    // Name
    entry->name = duplicate_string(name); 

    // From
    entry->fromHierarchy = fromHierarchy;

    // Absolute path
    if (entry->fileType != SYMLINK) {
        entry->absolutePath = realpath(entry->relativePath, NULL);
        NULL_CHECK(entry->absolutePath, "realpath");
    }
    else entry->absolutePath = get_absolute_path(info->exeDir, entry->relativePath);

    // Relative path in relation to hierarchy
    size_t hierLen = (fromHierarchy == HIER_A) ? info->lenA : info->lenB;
    entry->relativeToHier = malloc((strlen(entry->absolutePath) - hierLen + 1) * sizeof(char));
    NULL_CHECK(entry->relativeToHier, "malloc");
    strcpy(entry->relativeToHier, entry->absolutePath + hierLen);

    return entry;
}

// Destroy an entry using appropriate memory deallocation
void entry_destroy(EntryInfo *entry) {
    free(entry->relativePath);
    free(entry->absolutePath);
    free(entry->name);
    free(entry->relativeToHier);
    free(entry);
}

// Returns true if given symlink points inside given hierarchy. False otherwise
int symlink_in_hierachy(char *symlink, char hierarchy) {
    char *filePath = realpath(symlink, NULL);
    if (filePath == NULL) return 0;

    int inHier;
    if (hierarchy == HIER_A) inHier = !strncmp(info->hierarchyA, filePath, info->lenA);
    else inHier = !strncmp(info->hierarchyB, filePath, info->lenB);

    free(filePath);

    return inHier;
}

// Returns true if 2 files are the same. False otherwise
int files_are_same(EntryInfo *entryA, EntryInfo *entryB) {
    // If they have different sizes, they are definitely different
    if (entryA->size != entryB->size) return 0;
    // If they are both empty, they are definitely the same
    else if (entryA->size == 0) return 1;
    
    // Compare files' content
    int fdA = open(entryA->relativePath, O_RDONLY);
    if (fdA == -1) {
        perror("open()");
        exit(EXIT_FAILURE);
    }
    int fdB = open(entryB->relativePath, O_RDONLY);
    if (fdB == -1) {
        perror("open()");
        exit(EXIT_FAILURE);
    }

    // Create 2 buffers that will be used to store the contents of files
    char bufA[BUFLEN];
    char bufB[BUFLEN];

    int areSame = 0;
    ssize_t n;
    while ((n = read(fdA, bufA, sizeof(bufA))) > 0) {
        fullread(fdB, bufB, n); // Assure that exactly "n" bytes are read from fileB too
        areSame = !memcmp(bufA, bufB, n);
        if (!areSame) break;
    }

    // Close everything
    if (close(fdA) == -1 || close(fdB) == -1) {
        perror("close()");
        exit(EXIT_FAILURE);
    }

    return areSame;
}

// Returns true if given symlink are the same. False otherwise
int symlinks_are_same(EntryInfo *entryA, EntryInfo *entryB) {
    // Check if they are the same by comparing their realpaths
    char *bufA = realpath(entryA->relativePath, NULL);
    NULL_CHECK(bufA, "realpath");
    char *bufB = realpath(entryB->relativePath, NULL);
    NULL_CHECK(bufB, "realpath");
    int areSame = !strcmp(bufA, bufB);
    free(bufA);
    free(bufB);
    return areSame;
}

// Return true if 2 entries are the same. False otherwise
int entries_are_same(EntryInfo *entryA, EntryInfo *entryB) {
    // Compare the entries according to their file type
    switch (entryA->fileType) {
        case REGFILE:
        case HARDLINK:
            return files_are_same(entryA, entryB);
        case DIRECTORY:
            return !strcmp(entryA->relativeToHier, entryB->relativeToHier);
        case SYMLINK:
            return symlinks_are_same(entryA, entryB);
        default:
            fprintf(stderr, "Unkown file type\n");
            exit(EXIT_FAILURE);
    }
}

// Copy from file 'from' to file 'to'
void copy_file(EntryInfo *fromEntry, char *to) {
    char *from = fromEntry->relativePath;
    int n, fromFd, toFd;
    char buffer[BUFLEN];

    // Create the new file
    if ((toFd = open(to, O_CREAT | O_EXCL | O_WRONLY, fromEntry->perms)) == -1) {
        // If another entry with the same name was already created, don't re-create
        if (errno == EEXIST) return;
        // If the parent directory was not copied, ignore this file
        if (errno == ENOTDIR) return;
        perror("open()");
        exit(EXIT_FAILURE);
    }
    
    // Open the old file
    if ((fromFd = open(from, O_RDONLY)) == -1) {
        perror("open()");
        exit(EXIT_FAILURE);
    }

    // Do the writing
    while ((n = read(fromFd, buffer, sizeof(buffer))) > 0) {
        fullwrite(toFd, buffer, n);
    }

    // Close everything
    if (close(fromFd) == -1 || close(toFd) == -1) {
        perror("close()");
        exit(EXIT_FAILURE);
    }
}

// Copy a symlink to the new hierarchy
void copy_symlink(EntryInfo *entry, char *newSymlink) {
    char linksTo[BUFLEN];
    ssize_t n;
    if ((n = readlink(entry->relativePath, linksTo, sizeof(linksTo) - 1)) == -1) {
        perror("readlink()");
        exit(EXIT_FAILURE);
    }
    linksTo[n] = '\0';
    // Create the corresponding (new) symlink in dirC
    if (symlink(linksTo, newSymlink) == -1) {
        if (errno != EEXIST) {
            perror("symlink()");
            exit(EXIT_FAILURE);
        }
    }
}

// Manage the copying of the hardlinks
void manage_hardlinks(EntryInfo *entry, char *destination) {
    // Try to locate the file with the same i-node in the AVL tree
    char *path = avl_find(info->avl_hardlinks, entry->inode);
    // If found, simply create a hardlink to the same disk-file
    if (path != NULL) {
        if (link(path, destination) == -1) {
            if (errno == EEXIST) return;
            perror("link()");
            exit(EXIT_FAILURE);
        }
    }
    // Otherwise, copy the file to the new dirC, and store its path
    // for future references to it (for hardlink-creation)
    else {
        copy_file(entry, destination);
        avl_insert(info->avl_hardlinks, entry->inode, destination);
    }
}

// Create an entry to the new hierarchy
void create_entry(EntryInfo *entry) {
    // Get the absolute path of the destination found in the new hierarchy
    char *destination = concatinate(info->hierarchyC, entry->relativeToHier);
    // Create the entry according to its file type
    switch (entry->fileType) {
        case REGFILE:
            copy_file(entry, destination);
            break;
        case DIRECTORY:
            if (mkdir(destination, entry->perms) == -1) {
                // If another entry with the same name was already created, don't create this directory
                if (errno == EEXIST) break;
                perror("mkdir()");
                exit(EXIT_FAILURE);
            }
            break;
        case HARDLINK:
            manage_hardlinks(entry, destination);
            break;
        case SYMLINK:
            copy_symlink(entry, destination);
            break;
        default:
            fprintf(stderr, "Unkown file type\n");
            exit(EXIT_FAILURE);
    }
    free(destination);
}