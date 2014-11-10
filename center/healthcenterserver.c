#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "centerfunc.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

char buf_recv[LINELEN];
char buf_send[LINELEN];
int byte_to_send;
send_available_t avai_msg;

int main(int argc, char *argv[]) {
	int rv;
	/*read user.txt*/
	rv = read_user_info();
	if(rv == 2) {
		exit(1);
	}
	/*read available info*/
	read_available_info();
	if(rv == 2) {
		exit(1);
	}
	/******test*********/
	//int i;
	//for(i = 0; i < num_slot; i++) {
	//	printf("index: %d, day: %s, time: %s, doc: %s, port: %d\n",time_slot[i].index,time_slot[i].day,time_slot[i].time,time_slot[i].doc,time_slot[i].port);
	//}
	/******************/
	/*create & bind socket*/
	if(create_socket() == 2) {
		fprintf(stderr, "center: failed to create & bind socket\n");
		exit(1);
	}

	/*fork to handle request separatly*/
	printf("center: wait for connections....\n");

	/*wait for the client connect*/
	if(listen(center_sockfd, BACKLOG)) {
		perror("center:listen");
		exit(1);
	}

	/*deal with multiple process*/
	struct sigaction sa;
	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if(sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("center:sigaction");
		exit(1);
	}

	/*waiting for connection*/
	while(1) {
		int rv;
		if((rv = setup_connect()) == 2) {
			break;
			//continue;
		}

		/*child process after successful setup*/
		if(!fork()) {
			int pid = getpid();
			close(center_sockfd);//don't need this anymore
			/*child info*/
			printf("center: I'm child process:%d to serve you\n", pid);
			printf("new_fd: %d\n", new_fd);

			/*receive from patient*/
			memset(buf_recv, 0, sizeof recv_msg);
			if((rv = recv_msg(buf_recv)) == 2) {
				exit(1);
			}
			if(rv == 1) {
				continue;
			}

			printf("center: I recv %d bytes msg\n",(int)strlen(buf_recv));
			/*authentication*/
			if((rv = authen(buf_recv)) == 2) {
				exit(1);
			}
			if(rv == 1) {
				/*send fail msg*/
				memset(buf_send, 0, sizeof buf_send);
				sprintf(buf_send, "failure");
				byte_to_send = strlen(buf_send);
				send_msg(buf_send, byte_to_send);
				exit(1);
			}
			if(rv == 0) {
				/*send suc msg*/
				memset(buf_send, 0, sizeof buf_send);
				sprintf(buf_send, "success");
				byte_to_send = strlen(buf_send);

				printf("center: I will send %d bytes to patient\n", byte_to_send);

				send_msg(buf_send, byte_to_send);
			}
			printf(".........center: ready for phase 2...........\n");
			/*wait for avaliability*/
			memset(buf_recv, 0, sizeof buf_recv);
			rv = recv_msg(buf_recv);
			if(rv == 0) {
				/*check whether is "available"*/
				char *ava = "available";
				rv = same_string(buf_recv, ava);
			} else {
				printf("center: received wrong request for available information\n");
				exit(1);
			}
			if(rv == 0) {
				printf("ready to send available msg to patient\n");
				/*send available infomation to patient*/
				/*create msg*/
				int i;
				memset(&avai_msg, 0, sizeof avai_msg);
				avai_msg.num_slot = num_slot;
				for(i = 0;i < num_slot; i++) {
					avai_msg.one_avai[i].index = time_slot[i].index;
					strncpy(avai_msg.one_avai[i].day, time_slot[i].day,strlen(time_slot[i].day)+1);
					strncpy(avai_msg.one_avai[i].time, time_slot[i].time,strlen(time_slot[i].time)+1);
					printf("I'm sending: index:%d, day: %s, time: %s\n",avai_msg.one_avai[i].index, avai_msg.one_avai[i].day, avai_msg.one_avai[i].time);
				}

				/*send to patient*/
				send_msg((char *)&avai_msg, sizeof avai_msg);

				/*check the available file again*/
				/*mark the rev in the file & close imm*/
				/*send not ava now msg*/
				/*loop back*/
			} else {
				printf("center: received wrong request for available information\n");
				exit(1);
			}
			close(new_fd);
			exit(0);
		}
		/**************************************/

		/*prepare for next connection*/
		close(new_fd);
	}
	return 0;
}
