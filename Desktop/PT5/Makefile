# Makefile de exemplo ... eliminar para entrega fibonacci

main: commandlinereader.o main.o list.o
	gcc -pthread -o main commandlinereader.o main.o list.o
	gcc -o par-shell-terminal par-shell-terminal.o list.o

list.o: commandlinereader.c list.c list.h
	gcc -Wall -g -c commandlinereader.c list.c par-shell-terminal.c

main.o: main.c commandlinereader.h list.h
	gcc -Wall -g -c main.c

fibonacci: fibonacci.c
	gcc -o fibonacci fibonacci.c

clean:
	rm -f *.o main 

	