#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <ctype.h>
#include <sys/unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sendfile.h>

#define MAXBUFF 4096
#define CMDBUFF 256

#define BACKLOG 30

#define SERVERMODE 0
#define CLIENTMODE 1

#define REGISTER 5001
#define CONNECT 5002
#define TERMINATE 5003
#define GET 5004
#define PUT 5005
#define UPDATE 5006

#define REGISTER_ACK 6001
#define CONNECT_ACK 6002
#define TERMINATE_ACK 6003
#define GET_ACK 6004
#define PUT_ACK 6005
#define UPDATE_ACK 6006

#define CONNECTLIMIT 3
#define FILETRANSFERINPROGRESS 1
#define FILETRANSFERNOTINPROGRESS 0


int run(int, short int);
int updateRegistry(struct sockaddr_in);
int printHelp();
int printCreator();
int handleCMDs(char*, struct sockaddr_in, short int, int);
int getSelfIP(char*);
int registerOrConnect(int ,char *,int,int);
int connectToPeerClient();
int terminate(int, int);
int downloadFile(int , char* , int);
int upDownFile(int , char* , int, char *);

int serializeList(int, char*, int);
struct myIPPortStruct formListStruct(char*, int, int);
int deSerialize(char*, int, int);
int getSelfHost(char*);
//void *get_in_addr(struct sockaddr);
/*
struct sockaddr {
   unsigned short   sa_family;
   char             sa_data[14];
};

struct in_addr {
   unsigned long s_addr;
};

struct sockaddr_in {
   short int            sin_family;
   unsigned short int   sin_port;
   struct in_addr       sin_addr;
   unsigned char        sin_zero[8];
};
*/
