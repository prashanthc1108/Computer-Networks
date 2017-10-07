#define _GNU_SOURCE
#include "myFunctions.h"
#include "servList.h"
extern int globPort;
extern fd_set readfds;
int globPort;
extern int Dv[];
extern int NH[];

//extern neighbourList_t *nHead;
extern serverList_t *sHead;
//extern neighbourList_t *nCurr;
extern serverList_t *sCurr;
extern int SERVERCOUNT;
extern int NEIGHBOURCOUNT;
extern int SELFNODEID;
extern char SELFIP[32];
extern int SELFPORT;
extern int SELFSOCK;
extern int PACKETSC;
extern int UPDATEINTERVAL;
extern int LASTUPDATETIME;
extern int STATUS;

int serializeList(char *buff){
	int i=0;
/*	serverList_t* ptr=sHead;


	sprintf(buff,"%d|%d|%s",NEIGHBOURCOUNT,SELFPORT,SELFIP);
	while(i<NEIGHBOURCOUNT){
			sprintf(buff+strlen(buff),"|%s:%d:%d:%d",ptr->serverDetails.IPAddress,ptr->serverDetails.port,ptr->serverDetails.serverID,ptr->serverDetails.cost);
			ptr= ptr->next;
			i++;
	}*/
	sprintf(buff+strlen(buff),"%d|",SELFNODEID);
        while(i<SERVERCOUNT){
			//if(i+1 == SELFNODEID){
			//	sprintf(buff+strlen(buff),"%d|%d|",i+1,INT_MAX);
			//}else{
                        	sprintf(buff+strlen(buff),"%d|%d|",i+1,Dv[i]);
			//}
			i++;
	}
	buff[strlen(buff)]=0;
	return 0;
}



int runDVAlgorithm(int nodeStatus){
	int i;
	serverList_t* current = sHead;
	int minCostToDest,newNextHop;

	for (i=0;i<SERVERCOUNT;i++){
		minCostToDest=INT_MAX;
		if (i+1 == SELFNODEID){
			continue;
		}else{
			current = sHead;
			if(sHead == NULL)
			{
				return NULL;
			}
			//while(current->next != NULL){
			while(current != NULL){
				if(current->serverDetails.isNeighbour == true && current->serverDetails.serverID!=SELFNODEID){
					if((current->serverDetails.neighbourDv[i]!=INT_MAX&&current->serverDetails.cost!=INT_MAX)&&((current->serverDetails.neighbourDv[i]+current->serverDetails.cost)<minCostToDest)){
						minCostToDest=current->serverDetails.neighbourDv[i]+current->serverDetails.cost;
						//current->serverDetails.nextHop=i;
						//current->serverDetails.nextHop=current->serverDetails.serverID;
						newNextHop=current->serverDetails.serverID;
					}
				}
				current = current->next;
			}
		}
//		if(nodeStatus == ACTIVE && minCostToDest<Dv[i]){
			Dv[i]=minCostToDest;
			NH[i]=newNextHop;
			//printf("updated DV table for node %d, val:%d\n",i+1,minCostToDest);
//		}
	}
	return 0;
}

int deserializeCostChange(char *buffer){
	char *cmdCopy;
	int neighbourID,neighbourCost;
	serverList_t* current = sHead;
	cmdCopy = strdupa(buffer);
	neighbourID=atoi(strsep(&cmdCopy,"|"));
	neighbourCost=atoi(strsep(&cmdCopy,"|"));


	//	PACKETSC++;
	if(sHead == NULL)
	{
		return NULL;
	}
	//while(current->next != NULL){
	while(current != NULL){
		if(current->serverDetails.serverID == SELFNODEID){
                        current = current->next;
			continue;
                }

		if(current->serverDetails.serverID == neighbourID){
			current->serverDetails.cost=neighbourCost;
			if(neighbourCost == INT_MAX){
				current->serverDetails.linkStatus = DISABLED;
			}else{
				current->serverDetails.linkStatus = ACTIVE;
			}
			current->serverDetails.expiryTime = (int)time(NULL)+(UPDATEINTERVAL*3);
			printf("updated cost from %d to %d with cost %d\n",SELFNODEID,neighbourID,neighbourCost);
			break;

		}else{
			current = current->next;
		}
	}
	runDVAlgorithm(DISABLED);

	return 0;

}

int deserializeToList(char *buffer,int nodeID){
	char *cmdCopy;
	//char *tokCopy;
	char *token;
//	char* temp;
	int i=0;
 //       char* pEnd;
//	char *pos1;
//	char *pos2;
//	int x=2;
//	char *nodeIDc;
//	int nodeID;
/*nodeIDc = (char*)malloc (sizeof(char));
	strncpy(nodeIDc,buffer,1);
	nodeID=atoi(nodeIDc);
free(nodeIDc);
*/
//	char fakeBuff[MAXBUFF+1];
	int neighbourID,neighbourCost;
	//char neighbourIDc[5],neighbourCostc[5];
	serverList_t* current = sHead;

//	printf("received from node:%d packet[%s]\n",nodeID,buffer);
	PACKETSC++;
	if(sHead == NULL)
	{
		return NULL;
	}
	//while(current->next != NULL){
	while(current != NULL){
		if(current->serverDetails.serverID == SELFNODEID){
			current = current->next;
			continue;
		}
		if(current->serverDetails.isNeighbour == true){
			if(current->serverDetails.serverID ==nodeID){
//printf("Updating neighbour node%d's DV:\n",nodeID);
				current->serverDetails.linkStatus=ACTIVE;
				current->serverDetails.expiryTime=(int)time(NULL)+(UPDATEINTERVAL*3);
				cmdCopy = strdupa(buffer);

				while((token = strsep(&cmdCopy,"|"))){
					if (strcmp(token,"")==0 || i>=10){
						runDVAlgorithm(ACTIVE);

						return 0;

					}
					if((i%2)==0){
						neighbourID=atoi(token);
					}else{
						neighbourCost=atoi(token);
			//			if(neighbourID!=nodeID){
							current->serverDetails.neighbourDv[neighbourID-1]=neighbourCost;
//printf("%d %d\n",neighbourID,current->serverDetails.neighbourDv[neighbourID-1]);
			//			}
					}
					i++;
					if (strcmp(token,"")==0 || i>=10){
						runDVAlgorithm(ACTIVE);

						return 0;

					}
					/*	tokCopy = strdupa(token);
						temp = strsep(&tokCopy,":");
					//neighbourID = atoi(temp);
					neighbourID = strtol(temp,&pEnd,10);
					temp = strsep(&tokCopy,":");
					//neighbourCost = atoi(temp);
					neighbourCost = strtol(temp,&pEnd,10);
					current->serverDetails.neighbourDv[neighbourID-1]=neighbourCost;*/
				} 

				/*				while(buffer+x!=NULL || x<=strlen(buffer)){
								pos1 = strstr(buffer+x,":");
								pos2 = strstr(buffer+x,"|");
								memcpy(neighbourIDc,buffer+x,pos1-buffer);
								x+=strlen(neighbourIDc)+1;
								memcpy(neighbourCostc,buffer+x,pos2-pos1);
								x+=strlen(neighbourCostc)+1;
								current->serverDetails.neighbourDv[atoi(neighbourIDc)]=atoi(neighbourCostc);
								}
								*/
			}else{
				current = current->next;
			}

		}else{
			current = current->next;
		}
	}
	runDVAlgorithm(ACTIVE);

	return 0;

}

int updateNeighbours(){
        char sendBuffer[MAXBUFF];
        struct sockaddr_in serverAddr;
        int nBytes;
        socklen_t addr_size;
        serverList_t* current = sHead;

	memset(sendBuffer,0,MAXBUFF);
	serializeList(sendBuffer);
	nBytes = strlen(sendBuffer)+1;

	serverAddr.sin_family = AF_INET;
	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

        if(sHead == NULL)
        {
                return -1;
        }

	//while(current->next != NULL){
	while(current != NULL){
		if(current->serverDetails.serverID == SELFNODEID){
			current = current->next;
			continue;
                }
		if((current->serverDetails.isNeighbour == true) && (current->serverDetails.linkStatus == ACTIVE)){
			if(current->serverDetails.cost != INT_MAX){
				serverAddr.sin_port = htons(current->serverDetails.port);
				serverAddr.sin_addr.s_addr = inet_addr(current->serverDetails.IPAddress);
				addr_size = sizeof serverAddr;
				sendto(SELFSOCK,sendBuffer,nBytes,0,(struct sockaddr *)&serverAddr,addr_size);
	//			current->serverDetails.expiryTime=(int)time(NULL)+(3*UPDATEINTERVAL);
			//	printf("sent update [%s] to node %d\n",sendBuffer,current->serverDetails.serverID);
				current = current->next;
			}else{
				current = current->next;
			}
		}else{
			current = current->next;
		}
	}
        return 0;
}


int shouldIUpdate(){
	if(LASTUPDATETIME+UPDATEINTERVAL<(int)time(NULL)){
		if(updateNeighbours() == -1){
			printf("Unable to fetch from neighbour List\n");
		}
		LASTUPDATETIME=(int)time(NULL);
	}
	return 0;
}

int checkIfConnectionTerminated(){
        serverList_t* current = sHead;

        if(sHead == NULL)
        {
                return NULL;
        }

	//while(current->next != NULL){
	while(current != NULL){
		if (current->serverDetails.serverID == SELFNODEID){
                        current = current->next;
			continue;
                }

		//if(current->serverDetails.isNeighbour == true) {
		if((current->serverDetails.isNeighbour == true) && (current->serverDetails.linkStatus == ACTIVE)){
			if(current->serverDetails.expiryTime<(int)time(NULL)){
				printf("Node %d hasn't responded, probably down. Updating DV\n",current->serverDetails.serverID);
				//Dv[(current->serverDetails.serverID) -1]=INT_MAX; //TODO or should I calculate??
				current->serverDetails.cost = INT_MAX; //TODO REQD??
				current->serverDetails.linkStatus = CRASHED;
				runDVAlgorithm(CRASHED);
				current = current->next;
			}else{
				current = current->next;
			}

		}else{
			current = current->next;
		}
	}	

	return 0;
}

int run(){
	int sockFD,i,ret;
	//int nBytes;
	char recvBuffer[MAXBUFF+1],command[CMDBUFF],cmdString[7],fakeBuffer[MAXBUFF+1];
	char *buffcpy;
	char *token;
	struct sockaddr_in serverAddr, clientAddr;
	socklen_t client_addr_size;
	int maxFD;
	//int neighbID;
	struct timeval pollTime;

	fd_set readfds,tempreadfds;

	sockFD = socket(PF_INET, SOCK_DGRAM, 0);

	SELFSOCK=sockFD;

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SELFPORT);
	serverAddr.sin_addr.s_addr = inet_addr(SELFIP);
	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

	bind(sockFD, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

        FD_ZERO(&readfds);
        FD_ZERO(&tempreadfds);

	FD_SET(sockFD, &readfds);
        FD_SET(0, &readfds);

        maxFD = sockFD;
        client_addr_size = sizeof(clientAddr);

        while(1){
		pollTime.tv_sec = POLLTIME_SEC;
		pollTime.tv_usec = POLLTIME_USEC;
                tempreadfds = readfds;
                ret = select(maxFD+1, &tempreadfds, NULL, NULL, &pollTime);
                if (ret < 0)
                {
                        printf("select error\n");
			exit(0);
                        //return ret;
                }
		if (ret == 0)
                {
		//	if(STATUS!=CRASHED){
				//printf("handle timeout\n");
				checkIfConnectionTerminated();
				shouldIUpdate();
				//return ret;
		//	}
                }
                if(FD_ISSET(0, &tempreadfds)){
		//	if(STATUS!=CRASHED){
				if(fgets(command, CMDBUFF, stdin)){
					ret = handleCMDs(command);
				}
		//	}
                        //return ret;
		}
                                                               
                for (i = 1 ; i <= maxFD ; i++){
                        if(FD_ISSET(i, &tempreadfds)){
//				FD_SET(dataSockfd, &readfds);
//				if (dataSockfd>maxFD){
//					maxFD=dataSockfd;
//				}
		//		if(STATUS==CRASHED){
		//			break;
		//		}
				bzero(recvBuffer,MAXBUFF+1);
				//nBytes = recvfrom(sockFD,recvBuffer,MAXBUFF,0,(struct sockaddr *)&clientAddr, &client_addr_size);
				recvfrom(sockFD,recvBuffer,MAXBUFF,0,(struct sockaddr *)&clientAddr, &client_addr_size);
                               // printf("Message[Size:%d]:%s\n",nBytes,recvBuffer);
/*				if(strcmp(recvBuffer,"Thanks Machi!!")==0){
					printf("Recved 'Thanks Machi' Not gonna send welcome\n");
				}else{
					bzero(sendBuffer,MAXBUFF);
					strcpy(sendBuffer,"Thanks Machi!!");
					nBytes = strlen(sendBuffer)+1;
					sendto(SELFSOCK,sendBuffer,nBytes,0,(struct sockaddr *)&clientAddr, client_addr_size);
				}*/
				strncpy(cmdString, recvBuffer, 6);
				if(strcmp(cmdString,"UPDATE|")==0){
					deserializeCostChange(recvBuffer+6);
				}else{
					//neighbIDc=malloc(sizeof(char));
					//strncpy(neighbIDc,recvBuffer,1);
					//deserializeToList(recvBuffer,atoi(neighbIDc));
					memcpy(fakeBuffer,recvBuffer,sizeof(recvBuffer));
					                        buffcpy = strdupa(fakeBuffer);

                        token = strsep(&buffcpy,"|");

					deserializeToList(recvBuffer+2,(atoi(token)));
					//free(neighbIDc);
				}
			}
		}

	}	
}



int updateNeighbourCost(int nodeID, int cost){
        serverList_t* current = sHead;
	char sendBuffer[MAXBUFF];
        struct sockaddr_in serverAddr;
        int nBytes;
        socklen_t addr_size;

        if(sHead == NULL)
        {
		return NULL;
	}
	//while(current->next != NULL){
	while(current != NULL){
		if(current->serverDetails.serverID == nodeID){
			if(current->serverDetails.linkStatus != CRASHED){
				current->serverDetails.cost = cost;
				if(cost==INT_MAX){
					current->serverDetails.linkStatus = DISABLED;
				}else{
					current->serverDetails.linkStatus = ACTIVE;
				}
				current->serverDetails.expiryTime = (int)time(NULL)*(3*UPDATEINTERVAL);
				//printf("Updated cost of link frmo %d to %d with a cost %d\n",SELFNODEID,nodeID,cost);
				memset(sendBuffer,0,MAXBUFF);
				sprintf(sendBuffer,"UPDATE|%d|%d",SELFNODEID,cost);

				nBytes = strlen(sendBuffer)+1;

				serverAddr.sin_family = AF_INET;
				memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
				serverAddr.sin_port = htons(current->serverDetails.port);
				serverAddr.sin_addr.s_addr = inet_addr(current->serverDetails.IPAddress);
				addr_size = sizeof serverAddr;
				sendto(SELFSOCK,sendBuffer,nBytes,0,(struct sockaddr *)&serverAddr,addr_size);
				//printf("sent update string[%s] to node %d\n",sendBuffer,nodeID);

				return 0;
			}else{
				printf("You are trying to update a node that has crashed\n");
				return -1;
			}

		}else{
			current = current->next;
		}
	}
	return -1;

}

int handleCMDs(char* command){
        char *cmdCopy;
        char *token[5];
        char* pEnd;
	int i=0,noOfTokens=0;
	//int ci;

	int updateNodeID,updateCost;

	//char sendBuffer[MAXBUFF];
	//struct sockaddr_in serverAddr;
	//int nBytes;
	//socklen_t addr_size;
	//printf("Recvd CMD:%s\n",command);

        cmdCopy = strdupa(command);
        cmdCopy[strcspn(cmdCopy, "\r\n")] = 0;
        while((token[i] = strsep(&cmdCopy," "))){
                if(i >= 5){
                        printf("Invalid input. Too many parameters\n");
                        return -1;
                }
                i++;
        }
        noOfTokens=i;
	if((strcasecmp(token[0],"update")==0)){
		if(noOfTokens == 4){
			if((strcmp(token[3],"inf"))==0){
				updateCost=INT_MAX;
			}else{
				updateCost=strtol(token[3],&pEnd,10);
			}
			updateNodeID=strtol(token[1],&pEnd,10);
			if(updateNodeID==SELFNODEID){
				updateNodeID=strtol(token[2],&pEnd,10);
			}
			if(isNeighbourCheck(updateNodeID)==true){
				updateNeighbourCost(updateNodeID,updateCost);
				runDVAlgorithm(DISABLED);
//				updateNeighbours();
			}else{
				printf("Server %d is not my neighbour\n",updateNodeID);
			}
		}
		else{
			printf("Invalid number of inputs for %s command\n",token[0]);
		}
	} else if((strcasecmp(token[0],"step")==0)){
		if(noOfTokens == 1){
			printf("got Step command\n");
			updateNeighbours();
		}else{
			printf("Invalid number of inputs for %s command\n",token[0]);
		}
	} else if((strcasecmp(token[0],"packets")==0)){
		if(noOfTokens == 1){
			printf("No. of DV Packets received since it was checked last time is:%d\n",PACKETSC);
			PACKETSC=0;
		}else{
			printf("Invalid number of inputs for %s command\n",token[0]);
		}
	} else if((strcasecmp(token[0],"disable")==0)){
		if(noOfTokens == 2){
			updateNodeID=strtol(token[1],&pEnd,10);
			if(isNeighbourCheck(updateNodeID)==true){
				//updateCost=INT_MAX;
				//updateDV(updateNodeID,updateCost);
			
				updateNeighbourCost(updateNodeID,INT_MAX);
				runDVAlgorithm(DISABLED);
			}else{
				printf("Server %d is not my neighbour\n",updateNodeID);
			}
		}
		else{
			printf("Invalid number of inputs for %s command\n",token[0]);
		}
	} else if((strcasecmp(token[0],"crash")==0)){
		if(noOfTokens == 1){
			printf("Simulating crash\n Terminating process...\n");
			//STATUS == CRASHED;
			//for(ci=0;ci<SERVERCOUNT;ci++){
			//	if(ci+1 == SELFNODEID){
			//		continue;
			//	}
			//	updateNeighbourCost(ci+1,INT_MAX);
			//}
			exit(0);
		}
		else{
			printf("Invalid number of inputs for %s command\n",token[0]);
		}
	} else if((strcasecmp(token[0],"display")==0)){
		if(noOfTokens == 1){
			printf("Display Routing Table\n");
			displayRoutingTable();
		}
		else{
			printf("Invalid number of inputs for %s command\n",token[0]);
		}
	} else{
		printf("Invalid INPUT.\n");
	}

	free(cmdCopy);





/*	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(4091);
	serverAddr.sin_addr.s_addr = inet_addr("128.205.36.8");
	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

	addr_size = sizeof serverAddr;

	nBytes = strlen(command)+1;

	sendto(SELFSOCK,command,nBytes,0,(struct sockaddr *)&serverAddr,addr_size);
*/


	return 0;
}

int getSelfIP(char* selfIP){
        char hostname[1024];
        struct hostent *he;
        struct in_addr **addr_list;
        hostname[1023] = '\0';
        gethostname(hostname, 1023);
        if ( (he = gethostbyname( hostname ) ) == NULL){
                return 0;
        }
        addr_list = (struct in_addr **) he->h_addr_list;
        sprintf(selfIP,"%s",inet_ntoa(*addr_list[0]));
        return 0;
}

void readConfigFile(char* cfgFile){
	FILE *fp;
	//int len = 0;
	//char* line = NULL;
	char line[256];
	int lineNo=1;
	char* pEnd;
	getSelfIP(SELFIP);	
	fp = fopen(cfgFile,"r");
	//while (getline(&line, &len, fp) != -1) {
	while (fgets(line,sizeof(line), fp)) {
		printf("%s",line);
		if (lineNo == 1){
			SERVERCOUNT = strtol(line,&pEnd,10);
		}else if (lineNo == 2){
			NEIGHBOURCOUNT = strtol(line,&pEnd,10);
		}else if (lineNo <= SERVERCOUNT+2 && lineNo>2){
			getServerDetails(line);
		//}else if (lineNo > SERVERCOUNT+2 && lineNo <=SERVERCOUNT+NEIGHBOURCOUNT+2){
		}else if (lineNo > SERVERCOUNT+2){
			getNeighbourDetails(line);
		}
		lineNo++;
	}
	return;
}

void getServerDetails(char* line){
        char *lineCopy;
        char *token[4];
        char* pEnd;
        int i=0;
	struct myServerDetails serverInfo;

        lineCopy = strdupa(line);

	lineCopy[strcspn(lineCopy, "\r\n")] = 0;
	while(i<3){
		token[i] = strsep(&lineCopy," ");
		i++;
	}
	serverInfo.serverID = strtol(token[0],&pEnd,10);
        strcpy(serverInfo.IPAddress,token[1]);
	serverInfo.port = strtol(token[2],&pEnd,10);
	serverInfo.cost = INT_MAX;
	if(strcmp(serverInfo.IPAddress,SELFIP)==0){
		SELFNODEID=serverInfo.serverID;
		SELFPORT=serverInfo.port;
		serverInfo.cost = 0;
		updateDV(serverInfo.serverID,0);
	}
	serverInfo.linkStatus = NOTNEIGHBOUR;
	serverInfo.isNeighbour = false;
	serverInfo.expiryTime = (int)time(NULL)+(3*UPDATEINTERVAL);
	addToServerList(serverInfo);

}

void getNeighbourDetails(char* line){
        char *lineCopy;
        char *token[4];
        char* pEnd;
        int i=0;
	int cost,nodeID;
	int ret=0;

        lineCopy = strdupa(line);

	lineCopy[strcspn(lineCopy, "\r\n")] = 0;
	while(i<3){
		token[i] = strsep(&lineCopy," ");
		i++;
	}
	SELFNODEID=strtol(token[0],&pEnd,10);
	nodeID = strtol(token[1],&pEnd,10);
	cost = strtol(token[2],&pEnd,10);
	ret = addNeighbourToServerList(nodeID,cost);
	//neighbourInfo.neighbourID = strtol(token[1],&pEnd,10);
	//neighbourInfo.cost = strtol(token[2],&pEnd,10);
	//addToNeighbourList(neighbourInfo);
	if (ret == -1){
		printf("Couldn't add the Neighbour %d of cost %d to List\n",nodeID,cost);
	}
	updateDV(nodeID,cost);
	NH[nodeID-1]=nodeID;
}

void setDV(int node, int cost){
	Dv[node-1]=cost;
}


void updateDV(int node, int cost){
//	if(cost<Dv[node-1]){
		Dv[node-1]=cost;
//	}
}


void printDV(){
	int i;
	for(i=0;i<SERVERCOUNT;i++){
		printf("%d:%d\n",i+1,Dv[i]);
	}
}

void formUpdatePacket(){
	
}
