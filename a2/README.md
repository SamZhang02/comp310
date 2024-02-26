# Assignment 2

## Description

In this assignment, we implement a paging system for memory management for our shell created in assignment 1.

## Code Structure

The code is based on the the given starter code, a few extra files were added:

```
page.c <- page struct and relevant functions
framestore.c <- framestore struct and relevant functions
lru.c <- timer for the LRU cache and relevant functions
```

The framestore is an array of Page*, which contains the 3 lines within the page and relevant
metadata about page ownership and availabilities.

## Running the Code

Before building the file, mount a docker instance and enter the shell.

```shell
make docker-build
make docker
```

The shell can be make with the makefile, with specifications of the framesize and varmemsize.

```shell
make myshell -framesize={framestore_size} -varmemsize={variable_memory_size}
```



