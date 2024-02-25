CC=gcc
CFLAGS = -D FRAMESIZE=$(framesize) -D VARMEMSIZE=$(varmemsize)
SOURCES = shell.c interpreter.c shellmemory.c kernel.c pcb.c ready_queue.c page.c framestore.c lru.c
OBJECTS = $(SOURCES:.c=.o)
fs = 18
vs = 10

myshell: $(SOURCES)
	$(CC) $(CFLAGS) -c -g $(SOURCES)
	$(CC) $(CFLAGS) -g -o myshell $(OBJECTS)

debug-shell: $(SOURCES)
	$(CC) $(CFLAGS) -DDEBUG -c -g $(SOURCES)
	$(CC) $(CFLAGS) -g -o myshell $(OBJECTS)

debug:
	make clean
	make debug-shell framesize=$(fs) varmemsize=$(vs)

clean:
	rm myshell; rm *.o

docker-build:
	docker build . -t 310 -f Dockerfile

docker:
	docker run --rm -it --mount type=bind,source=.,target=/code --entrypoint /bin/bash -w /code 310

run:
	make clean
	make myshell framesize=$(fs) varmemsize=$(vs)
	./myshell

