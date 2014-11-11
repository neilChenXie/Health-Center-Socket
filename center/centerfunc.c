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
int num_slot;
uname user[MAXUSER];
upass pass[MAXUSER];
available_t time_slot[MAXSLOT];
int new_fd;
send_available_t avai_msg;

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
 *sigchld_handler
 * */
void sigchld_handler(int s) {
	while(waitpid(-1, NULL, WNOHANG) > 0);
}
/*
 *same_string
 * */
int same_string(char *s1, char *s2) {
	printf("compare %s and %s\n", s1, s2);
	int i;
	int len;
	if(strlen(s1) != strlen(s2)) {
		fprintf(stderr, "not the same length\n");
		return 2;
	}

	len = strlen(s1);
	for(i = 0; i < len; i++) {
		if(s1[i] != s2[i]) {
			fprintf(stderr, "2 string not the same\n");
			return 2;
		}
	}
	return 0;
}
/*read user.txt file
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
/*read available.txt file
 *rv: 0 for success, 2 for fail
 * */
int read_available_info() {
	FILE *fp;
	char line[LINELEN];
	char *sp;
	char *ep;

	fp = fopen("available.txt","r");
	if(fp == NULL) {
		fprintf(stderr, "center: cannot read available.txt\n");
		return 2;
	}

	num_slot = 0;
	while(fgets(line, sizeof(line), fp)) {
		memset(&time_slot[num_slot], 0, sizeof time_slot[num_user]);
		/*record index*/
		sp = line;
		ep = strchr(sp,' ');
		if(ep != NULL) {
			*ep = '\0';
		} else {
			fprintf(stderr, "center: wrong available.txt file fomat\n");
			return 2;
		}
		time_slot[num_slot].index = atoi(sp);
		/*record day*/
		sp = ep+1;
		ep = strchr(sp,' ');
		if(ep != NULL) {
			*ep = '\0';
		} else {
			fprintf(stderr, "center: wrong available.txt file fomat\n");
			return 2;
		}
		strncpy(time_slot[num_slot].day, sp, sizeof time_slot[num_slot].day);
		/*record time*/
		sp = ep+1;
		ep = strchr(sp,' ');
		if(ep != NULL) {
			*ep = '\0';
		} else {
			fprintf(stderr, "center: wrong available.txt file fomat\n");
			return 2;
		}
		strncpy(time_slot[num_slot].time, sp, sizeof time_slot[num_slot].time);
		/*record doc*/
		sp = ep+1;
		ep = strchr(sp,' ');
		if(ep != NULL) {
			*ep = '\0';
		} else {
			fprintf(stderr, "center: wrong available.txt file fomat\n");
			return 2;
		}
		strncpy(time_slot[num_slot].doc, sp, sizeof time_slot[num_slot].doc);
		/*record port*/
		sp = ep+1;
		ep = strchr(sp,'\n');
		if(ep != NULL) {
			*ep = '\0';
		} else {
			fprintf(stderr, "center: wrong available.txt file fomat\n");
			return 2;
		}
		time_slot[num_slot].port = atoi(sp);

		/*for next line*/
		num_slot++;
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
/*
 *receive msg
 * */
int recv_msg(char *buf) {
	int num_bytes;

	num_bytes = recv(new_fd, buf, LINELEN-1, 0);
	if(num_bytes == -1) {
		perror("center:recv");
		return 2;
	}
	if(num_bytes == 0) {
		return 1;
	}

	buf[num_bytes] = '\0';
	return 0;
}
/*
 *send msg
 * */
int send_msg(char *buf, int num_bytes) {

	//check_packet(buf);
	if(send(new_fd, buf, num_bytes, 0) == -1) {
		perror("center:send");
		return 2;
	}
	return 0;
}
/*
 *authen
 *rv: 0 for success, 1 for fail, 2 for not authen_msg
 * */
int authen(char *buf) {
	//printf("I will verify msg: %s\n",buf);
	char *sp;
	char *up;
	char *pp;
	int i;
	/*verify the msg for authen*/
	sp = strstr(buf,"authenticate");
	if(sp == NULL) {
		fprintf(stderr, "this is NOT authenticate message\n");
		return 2;
	}
	if(sp != NULL) {
		/*get username*/
		if((up = strchr(sp, ' ')) != NULL) {
			up++;
		} else {
			fprintf(stderr, "there is no username\n");
			return 2;
		}
		/*get password*/
		pp = strchr(up, ' ');
		if(pp != NULL) {
			*pp = '\0';
			pp++;
		} else {
			fprintf(stderr, "there is no password\n");
			return 2;
		}

		/*verify the uname and passw*/
		for(i = 0; i < num_user; i++) {
			if(same_string(user[i].username, up) == 0) {
				if(same_string(pass[i].userpass, pp) == 0) {
					return 0;
				}
			}
		}
	}
	return 1;
}

/*create avai_msg packet*/
int create_avai_msg(send_available_t *avai_msg) {
	int i;
	avai_msg->num_slot = num_slot;
	for(i = 0;i < num_slot; i++) {
		avai_msg->one_avai[i].index = time_slot[i].index;
		strncpy(avai_msg->one_avai[i].day, time_slot[i].day,strlen(time_slot[i].day));
		strncpy(avai_msg->one_avai[i].time, time_slot[i].time,strlen(time_slot[i].time));
	}
	return 0;
}

/*get selection
 *rv: num of selection -1 for failure
 * */
int get_selection(char *buf) {
	int res;
	char *sp;
	
	if(strstr(buf,"selection") != NULL) {
		sp = strchr(buf, ' ');
		if(sp != NULL) {
			sp++;
			res = atoi(sp);
			return res;
		} else {
			fprintf(stderr, "the selection msg is in wrong format\n");
			return -1;
		}
	} else {
		fprintf(stderr, "this infomation is not for selection\n");
		return -1;
	}
}
