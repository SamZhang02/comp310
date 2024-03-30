# Assignment 2

Sam Zhang 261072449

Bohan Wang 261023725

## Description

In this assignment, we implement some file system utils function for our shell.

Most new codes reside in `fsutil2.c`.

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

and launch the shell attaching your desired disk file, e.g.

```shell
./myshell blank.dsk
```



