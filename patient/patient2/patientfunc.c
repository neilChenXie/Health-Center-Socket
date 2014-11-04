#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "patientfunc.h"

int p_sockfd;
char uname[UNAMELEN];
char upass[UPASSLEN];
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
 *read patient file
 * */
int read_patient_info() {
	FILE *fp;
	char line[LINELEN];
	char *unfomat= "patient";
	char *up;
	char *pp;
	char *ep;

	fp = fopen(MY_INFO_FILE, "r");
	if(fp == NULL) {
		fprintf(stderr, "patient: cannot get my infomation\n");
		return 2;
	}

	while(fgets(line, sizeof line, fp)) {
		memset(uname, 0, UNAMELEN);
		memset(upass, 0, UPASSLEN);
		if((up = strstr(line, unfomat)) == NULL) {
			fprintf(stderr, "patient: wrong file fomat\n");
			return 2;
		}
		if((pp = strchr(up, ' ')) == NULL) {
			fprintf(stderr, "patient: no password found\n");
			return 2;
		}
		*pp = '\0';
		pp++;
		if((ep = strchr(pp, '\n')) != NULL) {
			*ep = '\0';
		}
		sprintf(uname, "%s", up);
		sprintf(upass, "%s", pp);
	}
	return 0;
}
/*
 *create socket
 *rv: 0 for success, 2 for fail
 * */
int create_connect_socket() {
	struct addrinfo hints, *serinfo;
	int rv;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	rv = getaddrinfo(NULL, MYPORT, &hints, &serinfo);
	if(rv == -1) {
		fprintf(stderr, "patient: %s\n",gai_strerror(rv));
		return 2;
	}

	p_sockfd = socket(serinfo->ai_family, serinfo->ai_socktype, serinfo->ai_protocol);
	if(p_sockfd == -1) {
		perror("patient:socket");
		return 2;
	}

	rv = connect(p_sockfd, serinfo->ai_addr, serinfo->ai_addrlen);

	if(rv == -1) {
		close(p_sockfd);
		perror("patient:connect");
		return 2;
	}

	inet_ntop(serinfo->ai_family, get_in_addr((struct sockaddr *)serinfo->ai_addr), s, sizeof s);
	printf("patient: connecting to %s\n", s);

	freeaddrinfo(serinfo);

	return 0;
}
/*
 *receive msg
 * */
int recv_msg(char *buf) {
	int num_bytes;

	num_bytes = recv(p_sockfd, buf, LINELEN-1, 0);
	if(num_bytes == -1) {
		perror("center:recv");
		return 2;
	}

	buf[num_bytes] = '\0';
	return 0;
}
/*
 *send msg
 * */
int send_msg(char *buf, int num_bytes) {

	printf("I will send %d bytes to center.\n", num_bytes);
	if(send(p_sockfd, buf, num_bytes, 0) == -1) {
		perror("center:send");
		return 2;
	}
	return 0;
}
