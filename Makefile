allocate: allocate.c
	gcc Wall -o -c allocate allocate.c

clean:
	rm -rf *.o