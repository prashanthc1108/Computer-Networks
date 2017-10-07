#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "myFunctions.h"
#include "servList.h"

#define clrscr() system("clear")

neighbourList_t *nHead = NULL;
serverList_t *sHead = NULL;
neighbourList_t *nCurr = NULL;
serverList_t *sCurr = NULL;

int Dv[]={INT_MAX,INT_MAX,INT_MAX,INT_MAX,INT_MAX};
int NH[]={0,0,0,0,0};
int NEIGHBOURCOUNT=0;
int SERVERCOUNT=0;
int SELFNODEID=0;
char SELFIP[32];
int SELFPORT;
int SELFSOCK;
int PACKETSC;
int UPDATEINTERVAL;
int LASTUPDATETIME;
//int STATUS=ACTIVE;

bool isValidOpt(const char *a, const char *b)
{
   if(strncmp(a, b, strlen(b)) == 0) return true;
   return false;
}

int main(int argc, char *argv[]){
	char* pEnd;
	char cfgFile[20];
	clrscr();

//char hostname[128];
//gethostname(hostname, sizeof hostname);
//printf("My hostname: %s\n", hostname);
//exit;






	if(argc == 3){
		if(isValidOpt(argv[1],"-t") || isValidOpt(argv[1],"-i")){
			if(isValidOpt(argv[1],"-t")){
				strcpy(cfgFile,argv[1]+2);
			}else{
				UPDATEINTERVAL = strtol(argv[1]+2,&pEnd,10);
				if(UPDATEINTERVAL <= 0 ){
					printf("Invalid routing-update-interval:%d(%s)\n",UPDATEINTERVAL,argv[1]);
					exit(0);
				}
			}
		}
		else{
			printf("First option is invalid. Correct Usage:\nEnter just 2 options:\n\t1) -t<topology-file-name>\n\t2) -i<routing-update-interval>\n");
			exit(0);
		}
                if(isValidOpt(argv[2],"-t") || isValidOpt(argv[2],"-i")){
			if(isValidOpt(argv[2],"-t")){
				strcpy(cfgFile,argv[2]);
			}else{
				UPDATEINTERVAL = strtol(argv[2]+2,&pEnd,10);
				if(UPDATEINTERVAL <= 0 ){
					printf("Invalid routing-update-interval:%d(%s)\n",UPDATEINTERVAL,argv[2]);
					exit(0);
				}
			}
                }
		else{
			printf("Second option is invalid. Correct Usage:\nEnter just 2 options:\n\t1) -t<topology-file-name>\n\t2) -i<routing-update-interval>\n");
			exit(0);
		}

	}
	else {
		printf("Too many or too few options supplied. Enter just 2 options:\n\t1) -t<topology-file-name>\n\t2) -i<routing-update-interval>\n");
		exit(0);
	}
	printf("Topop:%s ; Interval:%d s\n",cfgFile,UPDATEINTERVAL);
	readConfigFile(cfgFile);
	
	printf("ServerList:\n============\n");
	printServerList();	
	//printf("NeighbList:\n============\n");
	//printNeighbourList();	
	printf("Dist Vect:\n===========\n");
	printDV();
	printf("SelfIP:%s SelfPort:%d SelfnodeID:%d\n",SELFIP,SELFPORT,SELFNODEID);
	printf("Initial Routing Table:\n======================\n");
	displayRoutingTable();
	LASTUPDATETIME = (int)time(NULL);
	run();
	
	return 0;
}

