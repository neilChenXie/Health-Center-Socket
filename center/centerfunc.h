#define CENTERPORT "63123"
#define BACKLOG 10 //pending connections queue will hold
#define UNAMELEN 20
#define UPASSLEN 20
#define LINELEN 100
#define MAXUSER 10
#define MAXSLOT 10

typedef struct {
	char username[UNAMELEN];
}uname;
typedef struct {
	char userpass[UPASSLEN];
}upass;
typedef struct {
	int index;
	char day[4];
	char time[5];
	char doc[5];
	int port;
}available_t;
typedef struct {
	int num_slot;
	available_t one_avai[MAXSLOT];
}send_available_t;

extern int center_sockfd;
/*time slot info*/
extern int num_slot;
extern available_t time_slot[MAXSLOT];
/*user info*/
extern int num_user;
extern uname user[MAXUSER];
extern upass pass[MAXUSER];
extern int new_fd;
extern char buf_recv[LINELEN];
extern char buf_send[LINELEN];
extern send_available_t avai_msg;
extern int byte_to_send;

/*read file*/
int read_user_info();
int read_available_info();
/*setup connection*/
int create_socket();
void sigchld_handler(int s);
int setup_connect();
/*communication*/
int recv_msg(char *buf);
int send_msg(char *buf, int num_bytes);
/*msg check*/
int authen(char *buf);
int same_string(char *s1, char *s2);
