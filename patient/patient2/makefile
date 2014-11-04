patient1: patient2.o patientfunc.o
	gcc -o patient2 -g patient2.o patientfunc.o
patient1.o: patient2.c patientfunc.h patientfunc.c
	gcc -g -c -Wall patient2.c
patientfunc.o: patientfunc.h patientfunc.c
	gcc -g -c -Wall patientfunc.c

clean:
	rm patient2 *.o
