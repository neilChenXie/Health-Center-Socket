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
char avai_buf[LINELEN];
int num_slot;

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
	/*send buf_recv*/
	memset(buf_send, 0, LINELEN-1);
	sprintf(buf_send,"authenticate %s %s", uname, upass);
	send_msg(buf_send, strlen(buf_send));
	/*recv authen res msg*/
	while(1) {
		memset(buf_recv, 0, LINELEN-1);
		recv_msg(buf_recv);
		printf("patient: I got %d bytes msg\n", (int)strlen(buf_recv));
		printf("patient: I got result:%s\n",buf_recv);
		if(strstr(buf_recv, "failure") != NULL) {
			fprintf(stderr, "patient: my login infomation is wrong\n");
			exit(1);
		}
		if(strstr(buf_recv, "success") != NULL) {
			break;
		}
	}

	/*for phase 2*/
	printf(".........patient: ready for phase 2..............\n");

	/*send ava req msg*/
	memset(buf_send, 0, LINELEN);
	sprintf(buf_send, "available");
	send_msg(buf_send, strlen(buf_send));

	/*rec form until end*/
	send_available_t *avai_msg;
	memset(buf_recv, 0, LINELEN);
	recv_msg(buf_recv);
	avai_msg = (send_available_t *) buf_recv;

	//check_packet((char *)avai_msg);

	/**show available**/
	int i;
	for(i = 0; i < avai_msg->num_slot; i++) {
		printf("available information: index: %d, day: %s, time: %s\n", avai_msg->one_avai[i].index, avai_msg->one_avai[i].day, avai_msg->one_avai[i].time);
	}

	/*wait to stdin*/
	while(1) {
		printf("please enter your choice: ");
		if(read_patient_select() == -1) {
			continue;
		} else {
			int flag = 0;
			for(i = 0; i < avai_msg->num_slot; i++) {
				if(avai_msg->one_avai[i].index == user_sel) {
					flag = 1;
					break;
				}
			}
			if(flag == 1) {
				break;
			}
		}
	}

	/*send index to center*/
	memset(buf_send, 0, sizeof buf_send);
	sprintf(buf_send, "selection %d", user_sel);
	send_msg(buf_send, strlen(buf_send));
	/*get result*/
	memset(buf_recv, 0, sizeof buf_recv);
	recv_msg(buf_recv);
	close(p_sockfd);
	/*check*/
	rv = check_res_sel(buf_recv);
	if(rv == -1) {
		exit(0);
	}
	printf("the port number of doctor is:%d\n", rv);
	/*suc to phase3*/
	/*send avaliable*/
	return 0;
}
