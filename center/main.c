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

char authen_msg[LINELEN];
char authen_res[LINELEN];
int byte_to_send;

int main(int argc, char *argv[]) {
	/*read user.txt*/
	read_user_info();
	/*test*/
	//int num;
	//for(num = 0; num < num_user; num++) {
	//	printf("user%d, username:%s, password:%s\n", num+1, user[num].username, pass[num].userpass);
	//}
	/******/
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

			while(1) {
				/*receive from patient*/
				if((rv = recv_msg(authen_msg)) == 2) {
					exit(1);
				}
				if(rv == 1) {
					continue;
				}
				printf("center: I recv %d bytes msg\n",(int)strlen(authen_msg));
				/*authentication*/
				if((rv = authen(authen_msg)) == 2) {
					continue;
				}
				if(rv == 1) {
					/*send fail msg*/
					sprintf(authen_res, "failure");
					byte_to_send = strlen(authen_res);
					send_msg(authen_res, byte_to_send);
				}
				if(rv == 0) {
					/*send suc msg*/
					sprintf(authen_res, "success");
					byte_to_send = strlen(authen_res);
					send_msg(authen_res, byte_to_send);
				}
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
