#Makefile - Version 2

CC = gcc

CFLAGS = -Wall -Wextra -std=c99

OBJ = main.o vma.o

all: build

build: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o vma

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

vma.o: vma.c
	$(CC) $(CFLAGS) -c vma.c

run_vma:
	./vma

clean:
	rm -f vma $(OBJ)