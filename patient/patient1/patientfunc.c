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
int user_sel;
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

/*
 *check bytes
 * */
void check_packet(char *buf) {
	uint8_t che[LINELEN];
	int i;

	memcpy(che, buf, LINELEN);
	printf("the packet is:\n");
	for(i = 0; i < LINELEN; i++) {
		printf("%02X",che[i]);
	}
	printf("\n");
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
/*read stdin
 *rv: -1 for error
 * */
int read_patient_select() {
	char line[LINELEN];
	if(fgets(line, LINELEN, stdin)) {
		//printf("my select is:%s\n", line);
		user_sel = atoi(line);
		return user_sel;
	} else {
		printf("no input at all\n");
		return -1;
	}
	return -1;
}
/*
 *create socket
 *rv: 0 for success, 2 for fail
 * */
int create_connect_socket() {
	struct addrinfo hints, *serinfo;
	int rv;
	//char s[INET6_ADDRSTRLEN];

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

	//inet_ntop(serinfo->ai_family, get_in_addr((struct sockaddr *)serinfo->ai_addr), s, sizeof s);
	//printf("patient: connecting to %s\n", s);

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
	//check_packet(buf);
	buf[num_bytes] = '\0';
	return 0;
}
/*
 *send msg
et * */
int send_msg(char *buf, int num_bytes) {

	if(send(p_sockfd, buf, num_bytes, 0) == -1) {
		perror("center:send");
		return 2;
	}
	return 0;
}
/*
 *check_res_sel
 rv: port for suc, -1 for failure
 * */
int check_res_sel(char *buf) {
	char *sp;
	int res;
	if(strstr(buf, "doc") != NULL) {
		sp = strchr(buf, ' ');
		if(sp != NULL) {
			sp++;
			res = atoi(sp);
			return res;
		} else {
			fprintf(stderr, "wrong fomat\n");
			return -1;
		}
	} else if(strstr(buf, "nonavailable") != NULL) {
		fprintf(stderr, "not available\n");
		return -1;
	} else {
		fprintf(stderr, "not reply for selection");
		return -1;
	}
}
