#include <stdio.h>
#include <stdlib.h>
#include <string.h>
char user[100];
char pass[100];

int main(int argc, char*argv[]) {
	FILE *fp;
	char buf[100];
	fp = fopen("users.txt","r+");
	while(fgets(buf, sizeof buf, fp) != 0) {
		char *sp;
		char *pp;
		char *ep;
		printf("the line is: %s\n",buf);
		sp = buf;
		pp = strchr(sp,' ');
		*pp = '\0';
		pp++;
		ep = strchr(pp,'\n');
		if(ep != NULL) {
			*ep = '\n';
		}
		sprintf(user, "%s",sp);
		sprintf(pass, "%s",pp);
		printf("the username is: %s\n", user);
		printf("the password is: %s\n", pass);
	}
	return 0;
}
// strstr()
