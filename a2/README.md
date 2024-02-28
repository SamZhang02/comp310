# Assignment 2

Sam Zhang 261072449

Bohan Wang 261023725

## Description

In this assignment, we implement a paging system for memory management for our shell created in assignment 1.

## Code Structure

The code is based on the the given starter code, a few extra files were added:

```c
page.c         // page struct and relevant functions
framestore.c   // framestore struct and relevant functions
lru.c          // timer for the LRU cache and relevant functions
```

The framestore is a singleton array of `Page*`, which contains the 3 lines within the page and relevant
metadata about page ownership and availabilities.

The code entrypoint is `shell.c`, where a shell is started with a running interpreter for user commands. 

Upon calls to `run` or `exec`, a backing store is created and the files being ran are copied into it. They are then being read from the backing line, with an initial loading of two pages into the framestore, and subsequently one page at a time as they are being read. 

## Running the Code

Before building the file, mount a docker instance and enter the shell.

```make
make docker-build
make docker
```

The shell can be make with the makefile, with specifications of the framesize and varmemsize.

```make
make myshell -framesize={framestore_size} -varmemsize={variable_memory_size}
```



