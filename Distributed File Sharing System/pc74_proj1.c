#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "genFunctions.h"
#include "registryList.h"

#define clrscr() system("clear")

node_t *head_server = NULL;
node_t *current_server = NULL;
node_t *head_client = NULL;
node_t *current_client = NULL;
bool isRegistered = false;
fd_set readfds;
int maxFD;

int main(int argc, char *argv[]){
	int ret=0, port=0;
	char* pEnd;
	short int runMode;
	clrscr();

	node_t * head = NULL;
//char hostname[128];
//gethostname(hostname, sizeof hostname);
//printf("My hostname: %s\n", hostname);
//exit;

	head = malloc(sizeof(node_t));
	if (head == NULL) {
		return 1;
	}




	if(argc == 3){
		port = strtol(argv[2],&pEnd,10);
		if (port <= 0 || port > 65535){
			printf("Invalid argument for port: %s\n", argv[2]);
			exit(0);
		}
		if (strcasecmp(argv[1],"s") == 0){
			runMode = 0;
		}
		else if (strcasecmp(argv[1],"c") == 0){
			runMode = 1;
                }
		else{
			printf("Invalid mode. Should be either [s]erver or [c]lient \n");
		}
		ret = run(port,runMode);
		if(ret == -1){//TODO handle all returns
			printf("Unable to start as %s. Err:%d\n", argv[1], ret);
		}

	}
	else {
		printf("Too many or too few arguments supplied. Enter just 2 arguments:\n\t1) Mode: [s]erver or [c]lient \n\t2) Port\n");
	}
	return 0;
}

