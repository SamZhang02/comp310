CC=gcc
CFLAGS=

myshell: shell.c interpreter.c shellmemory.c
	$(CC) $(CFLAGS) -c shell.c interpreter.c shellmemory.c
	$(CC) $(CFLAGS) -o myshell shell.o interpreter.o shellmemory.o

clean: 
	rm myshell; rm *.o

docker:
	docker run --rm -it --mount type=bind,source=.,target=/code --entrypoint /bin/bash -w /code gcc:13.2
