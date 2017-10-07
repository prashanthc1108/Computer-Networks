#include <stdbool.h>

struct myIPPortStruct {
	char IPAddress[16];
	char hostName[32];
	char fileName[64];
	int port;
	int sockfd;
    	int runMode;
};

typedef struct node {
    struct myIPPortStruct hostAddr;
    struct node * next;
} node_t;

int printHostList(int);
int addToHostList(struct myIPPortStruct, int);
bool isHostListEmpty(int);
int removeFromHostList(int,int);
int lengthOfHostList(int);
struct node* find(int, int);
struct node* findIPinList(char*, int);
struct node* findHostinList(char*, int);
int removeFromHostListSockfd(int, int);
