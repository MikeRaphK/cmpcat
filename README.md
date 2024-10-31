# cmpcat - Directory Hierarchy Comparison and Merging Tool

`cmpcat` is a command-line utility for comparing two directory hierarchies and optionally merging them into a new structure. It efficiently compares differences in files, directories, and links between two specified directories, highlighting the distinctions and handling file types and links in a memory-efficient manner. The merging function can also consolidate these differences into a new directory without redundancy.

### Features
* Compare Directory Hierarchies: Identifies differences in files, directories, and links between two directories.
* Merge Directories: Combines two directory structures into a new directory while maintaining unique entries and eliminating duplicates.
* Efficient Memory Management: Ensures zero memory leaks or errors. Supports Valgrind testing for memory integrity.
* Hardlink and Symlink Handling: Manages hardlinks and symlinks with a custom AVL tree for efficient hardlink referencing.

### Compilation

The project includes a Makefile to simplify compilation:

```bash
make            # Compiles the project
make clean      # Removes compiled files
make count      # Counts source lines of code
```

### Execution

Use the following command-line options:

* Compare Directories:

```bash
./cmpcat -d pathTo/dirA pathTo/dirB
```

* Merge Directories (optional -s flag to specify output directory):

```bash
./cmpcat -d pathTo/dirA pathTo/dirB -s pathTo/output
```

Both relative and absolute paths are supported, and all paths must end with a /.

### Test Cases

A compressed test file *datatar.tar* is included, containing various scenarios to validate the tool’s functionality. Untar it using:

```bash
tar xf datatar.tar
```

### File Structure
- src/: Source files.
- include/: Header files.
- build/: Generated object files.
- datatar.tar: Test directories for verifying the program's functionality.

### University Project
This project was developed as part of the Operating Systems course (course professor: Alex Delis) at the National and Kapodistrian University of Athens (NKUA). It fulfills the assignment requirements for Programming Assignment #4 and demonstrates concepts related to file management, memory optimization, and system calls in a Unix-like environment. The project received a grade of 100/100 and excellent comments.

### Authors
* [Michael-Raphael Kostagiannis](https://github.com/MikeRaphK)
* [Giorgos Sofronas](https://github.com/gsofron)
