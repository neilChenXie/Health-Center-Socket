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

int center_sockfd;
int num_user;
uname user[MAXUSER];
upass pass[MAXUSER];
int new_fd;

/**********************private functions*****************************/
/*
 *get_in_addr
 * */
void *get_in_addr(struct sockaddr *sa) {
	if(sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	} else {
		return &(((struct sockaddr_in6*)sa)->sin6_addr);
	}
}

/**********************public functions******************************/
/*
 *sigchld_handler
 * */
void sigchld_handler(int s) {
	while(waitpid(-1, NULL, WNOHANG) > 0);
}
/*read file
 *rv: 0 for success, 2 for fail
 * */
int read_user_info() {
	FILE *fp;
	char line[LINELEN];
	char *uname = "patient";
	char *passw = "password";
	char *up;//for username
	char *pp;//for password
	char *ep;

	fp = fopen("users.txt","r");
	if(fp == NULL) {
		fprintf(stderr, "center: cannot read user.txt\n");
		return 2;
	}

	num_user = 0;
	while(fgets(line, sizeof(line), fp)) {
		if(((up = strstr(line, uname)) != NULL) && (pp = strstr(line, passw)) != NULL) {
			/*test*/
			//printf("line is: %s\n", line);
			/*record username*/
			memset(user[num_user].username, 0, UNAMELEN);
			if((ep = strchr(up, ' ')) != NULL) {
				*ep = '\0';
			}
			sprintf(user[num_user].username,"%s",up);
			/*record password*/
			memset(pass[num_user].userpass, 0, UPASSLEN);
			if((ep = strchr(pp, '\n')) != NULL) {
				*ep = '\0';
			}
			sprintf(pass[num_user].userpass,"%s",pp);
			/*next*/
			num_user++;
		} else {
			/*not right format*/
			continue;
		}
	}
	return 0;
}
/*create socket
 *rv: 0 for success, 2 for fail
 * */
int create_socket() {
	struct addrinfo hints, *res;
	int rv;
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if((rv = getaddrinfo(NULL, CENTERPORT, &hints, &res)) != 0) {
		fprintf(stderr, "%s\n",gai_strerror(rv));
		return 2;
	}

	center_sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(center_sockfd == -1) {
		perror("center: socket");
		return 2;
	}
	
	if(bind(center_sockfd, res->ai_addr, res->ai_addrlen) == -1) {
		close(center_sockfd);
		perror("center:bind");
		return 2;
	}
	freeaddrinfo(res);
	return 0;
}
/*
 *set up connection
 *rv 0 for success, 2 for fail
 * */
int setup_connect() {
		struct sockaddr_storage their_addr;
		socklen_t sin_size;
		char s[INET6_ADDRSTRLEN];

		sin_size = sizeof their_addr;
		new_fd = accept(center_sockfd, (struct sockaddr*)&their_addr, &sin_size);
		if(new_fd == -1) {
			perror("center:accept");
			return 2;
		}
		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		printf("center: got connection from %s\n", s);
		return 0;
}
