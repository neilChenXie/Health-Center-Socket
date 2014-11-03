center: main.o centerfunc.o
	gcc -o center -g main.o centerfunc.o
main.o: main.c centerfunc.h centerfunc.c
	gcc -g -c -Wall main.c
centerfunc.o: centerfunc.h centerfunc.c
	gcc -g -c -Wall centerfunc.c

clean: 
	rm center *.o
