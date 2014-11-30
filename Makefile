CFLAGS=-ggdb -std=c99 -Wall
CC=clang

all : functions.o main.c
	$(CC) $(CFLAGS) main.c functions.o -o exercise

functions : functions.c functions.h
	$(CC) $(CFLAGS) functions.c -c  

clean: 
	rm -f *.o
	rm -f exercise

