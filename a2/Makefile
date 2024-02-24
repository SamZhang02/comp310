CC=gcc
CFLAGS=
SOURCES = shell.c interpreter.c shellmemory.c kernel.c pcb.c ready_queue.c page.c framestore.c
OBJECTS = $(SOURCES:.c=.o)

myshell: $(SOURCES)
	$(CC) $(CFLAGS) -D FRAMESIZE=$(framesize) -D VARMEMSIZE=$(varmemsize) -c -g $(SOURCES)
	$(CC) $(CFLAGS) -o myshell $(OBJECTS)

debug-shell: $(SOURCES)
	$(CC) $(CFLAGS) -D FRAMESIZE=$(framesize) -D VARMEMSIZE=$(varmemsize) -c -g $(SOURCES)
	$(CC) $(CFLAGS) -g -o myshell $(OBJECTS)

debug:
	make clean
	make debug-shell framesize=100 varmemsize=100

clean:
	rm myshell; rm *.o

docker-build:
	docker build . -t 310 -f Dockerfile

docker:
	docker run --rm -it --mount type=bind,source=.,target=/code --entrypoint /bin/bash -w /code 310

run:
	make clean
	make myshell framesize=100 varmemsize=100
	./myshell

