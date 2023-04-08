allocate: allocate.c
	gcc -Wall -g -o allocate allocate.c -lm

clean:
	rm -rf *.o