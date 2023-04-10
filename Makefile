allocate: allocate.c helper.o process.h
	gcc -Wall -g -o allocate allocate.c helper.o -lm

helper.o: helper.c helper.h
	gcc -Wall -g -c helper.c -lm

clean:
	rm -rf *.o allocate
