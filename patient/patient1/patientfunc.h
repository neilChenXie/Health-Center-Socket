/*different patient should change this*/
#define MY_INFO_FILE "patient1.txt"

#define MYPORT "63123"
#define UNAMELEN 20
#define UPASSLEN 20
#define LINELEN 1000
#define MAXSLOT 10

typedef struct {
	int index;
	char day[4];
	char time[5];
	char doc[5];
	int port;
	int use_flag;
}available_t;
typedef struct {
	int num_slot;
	available_t one_avai[MAXSLOT];
}send_available_t;

extern int p_sockfd;
extern char uname[UNAMELEN];
extern char upass[UPASSLEN];
extern char buf_send[LINELEN];
extern char buf_recv[LINELEN];
extern int num_slot;

int read_patient_info();
int create_connect_socket();
int send_msg(char *buf, int num_bytes);
int recv_msg(char *buf);
