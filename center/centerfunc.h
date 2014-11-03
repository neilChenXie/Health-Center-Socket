#define CENTERPORT "63123"
#define BACKLOG 10 //pending connections queue will hold
#define UNAMELEN 20
#define UPASSLEN 20
#define LINELEN 100
#define MAXUSER 10
typedef struct {
	char username[UNAMELEN];
}uname;
typedef struct {
	char userpass[UPASSLEN];
}upass;

extern int center_sockfd;
extern int num_user;
extern uname user[MAXUSER];
extern upass pass[MAXUSER];
extern int new_fd;

int read_user_info();
int create_socket();
void sigchld_handler(int s);
//void *get_in_addr(struct sockaddr *sa);
int setup_connect();
