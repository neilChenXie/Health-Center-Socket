#define MY_INFO_FILE "patient2.txt"
#define MYPORT "63123"
#define UNAMELEN 20
#define UPASSLEN 20
#define LINELEN 100

extern int p_sockfd;
extern char uname[UNAMELEN];
extern char upass[UPASSLEN];
extern char buf_send[LINELEN];
extern char buf_recv[LINELEN];

int read_patient_info();
int create_connect_socket();
int send_msg(char *buf, int num_bytes);
int recv_msg(char *buf);
