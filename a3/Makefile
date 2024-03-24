OBJECTS=linked_list.o shell.o pcb.o kernel.o cpu.o interpreter.o shellmemory.o fs/block.o fs/debug.o fs/directory.o fs/file.o fs/filesys.o fs/free-map.o fs/fsutil.o fs/inode.o fs/list.o fs/ide.o fs/partition.o fs/bitmap.o fs/cache.o fs/fsutil2.o

define cc-command
gcc -g -c -Wall -D FRAME_STORE_SIZE=$(framesize) -D VAR_STORE_SIZE=$(varmemsize) $< -o $@
endef

all: myshell

$(OBJECTS): %.o: %.c
	$(cc-command)

myshell: $(OBJECTS)
	gcc -o myshell $(OBJECTS)

clean: 
	rm *.o
	rm fs/*.o
	rm myshell

build-docker:
	docker build . -t 310 -f Dockerfile

docker:
	docker run --rm -it --mount type=bind,source=.,target=/code --entrypoint /bin/bash -w /code 310

# run with a clean and empty disk
run-clean:
	make myshell framesize=999 varmemsize=999 
	cp clean_disks/blank.dsk ./clean.dsk
	./myshell ./clean.dsk -f