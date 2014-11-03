#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include "patientfunc.h"

char buf_send[LINELEN];
char buf_recv[LINELEN];

int main(int argc, char *argv[]) {
	int rv;
	/*read file*/
	if((rv = read_patient_info()) == 2) {
		exit(1);
	}
	/*test*/
	//printf("my infomation: user: %s, pass: %s\n", uname, upass);
	/******/
	/*create socket and connect*/
	if((rv = create_connect_socket()) == 2) {
		exit(1);
	}
	/*send authen_msg*/
	memset(buf_send, 0, LINELEN-1);
	sprintf(buf_send,"test to test");
	send_msg(buf_send, strlen(buf_send));
	/*recv authen res msg*/
	memset(buf_recv, 0, LINELEN-1);
	recv_msg(buf_recv);
	printf("patient: I got result:%s\n",buf_recv);

	return 0;
}
