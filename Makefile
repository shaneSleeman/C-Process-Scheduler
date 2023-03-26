allocate: allocate.c
	gcc -Wall -g -o allocate allocate.c

clean:
	rm -rf *.o