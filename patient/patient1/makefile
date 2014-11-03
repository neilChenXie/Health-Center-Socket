patient1: patient1.o patientfunc.o
	gcc -o patient1 -g patient1.o patientfunc.o
patient1.o: patient1.c patientfunc.h patientfunc.c
	gcc -g -c -Wall patient1.c
patientfunc.o: patientfunc.h patientfunc.c
	gcc -g -c -Wall patientfunc.c

clean:
	rm patient1 *.o
