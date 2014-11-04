center: healthcenterserver.o centerfunc.o
	gcc -o center -g healthcenterserver.o centerfunc.o
healthcenterserver.o: healthcenterserver.c centerfunc.h centerfunc.c
	gcc -g -c -Wall healthcenterserver.c
centerfunc.o: centerfunc.h centerfunc.c
	gcc -g -c -Wall centerfunc.c

clean: 
	rm center *.o
