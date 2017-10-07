#include <stdbool.h>
#include <limits.h>
#include <time.h>

#define true  1
#define false 0

#define ACTIVE 	 0
#define CRASHED  1
#define DISABLED 2
#define NOTNEIGHBOUR 3

struct myServerDetails {
	int serverID;
	char IPAddress[32];
	int port;
	int cost;
	int neighbourDv[5];
	int isNeighbour;
	int linkStatus;
	int expiryTime;
	int nextHop;
	//int nextHop[5];
};

struct myNeighbourDetails {
	int neighbourID;
	int cost;
	int neighbourDv[5];
};

typedef struct serverList {
        struct myServerDetails serverDetails;
	struct serverList *next;
} serverList_t;


typedef struct neighbourList {
        struct myNeighbourDetails neighbourDetails;
	struct neighbourList *next;
} neighbourList_t;

int displayRoutingTable();
int printServerList();
int printNeighbourList();
int addToServerList(struct myServerDetails );
int addNeighbourToServerList(int,int);
int addToNeighbourList(struct myNeighbourDetails );
int isNeighbourCheck(int);
