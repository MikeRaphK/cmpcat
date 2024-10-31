#ifndef ENTRY_MANAGER_H
#define ENTRY_MANAGER_H

#define REGFILE   'R'
#define DIRECTORY 'D'
#define HARDLINK  'H'
#define SYMLINK   'S'

#define HIER_A 'A'
#define HIER_B 'B'
#define HIER_C 'C'

#include <sys/types.h>  // ino_t etc.

// Save all useful information about an entry over here
// NOTE: This is done to avoid multiple calls of lstat() and
// avoid passing dirents as arguements between functions
typedef struct {
    char *relativePath;     // Relative path of entry in relation to the executable     
    char *relativeToHier;   // Relative path of entry in relation to parent hierarchy
    char *absolutePath;     // Absolute path of entry
    char *name;             // Name of entry
    ino_t inode;            // Inode id
    off_t size;             // Size of entry
    time_t mtime;           // Modification time
    mode_t perms;           // Entry permissions
    char fileType;          // File type of entry. Uses #defines listed above
    char fromHierarchy;     // Indicates from which hierarchy this entry comes from. Uses #defines listed above
} EntryInfo;

// Initialize an entry
EntryInfo *entry_init(char *relativePath, char *name, char fromHierarchy);

// Destroy an entry using appropriate memory deallocation
void entry_destroy(EntryInfo *entry);

// Returns true if given symlink points inside given hierarchy. False otherwise
int symlink_in_hierachy(char *symlink, char hierarchy);

// Returns true if 2 files are the same. False otherwise
int files_are_same(EntryInfo *entryA, EntryInfo *entryB);

// Returns true if given symlink are the same. False otherwise
int symlinks_are_same(EntryInfo *entryA, EntryInfo *entryB);

// Return true if 2 entries are the same. False otherwise
int entries_are_same(EntryInfo *entryA, EntryInfo *entryB);

// Copy from file 'from' to file 'to'
void copy_file(EntryInfo *fromEntry, char *to);

// Copy a symlink to the new hierarchy
void copy_symlink(EntryInfo *entry, char *newSymlink);

// Manage the copying of the hardlinks
void manage_hardlinks(EntryInfo *entry, char *destination);

// Create an entry to the new hierarchy
void create_entry(EntryInfo *entry);

#endif