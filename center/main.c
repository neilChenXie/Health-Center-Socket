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
		rv = setup_connect();

		if(rv == 2) {
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

            //if (send(new_fd, "Hello, world!", 13, 0) == -1)
            //    perror("send");
            //close(new_fd);
			exit(0);
		}
		/**************************************/

		/*prepare for next connection*/
		close(new_fd);
	}
	return 0;
}
