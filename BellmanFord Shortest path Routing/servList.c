#include "servList.h"
#include "myFunctions.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

extern neighbourList_t *nHead;
extern int SERVERCOUNT;
extern serverList_t *sHead;
extern neighbourList_t *nCurr;
extern serverList_t *sCurr;
extern int SELFNODEID;
extern int Dv[];
extern int NH[];

int displayRoutingTable(){
	int i;
	//serverList_t *ptr = sHead;
//	int i=1;
	printf("ID:     NextHop:  Cost:\n=======================\n");
/*	while(ptr != NULL)
	{
		if(ptr->serverDetails.serverID==SELFNODEID){
			ptr = ptr->next;
		//	i++;
			continue;
		}*/
	/*	if(ptr->serverDetails.cost==INT_MAX){
			printf("%8d%10d%8s\n", ptr->serverDetails.serverID, ptr->serverDetails.nextHop, "INF");
		}else{
			printf("%8d%10d%8d\n", ptr->serverDetails.serverID, ptr->serverDetails.nextHop, ptr->serverDetails.cost);
		}*/
/*		printf("%8d%10d%8d\n", ptr->serverDetails.serverID, NH[ptr->serverDetails.serverID-1], Dv[ptr->serverDetails.serverID-1]);
		ptr = ptr->next;
	}*/
	for(i=0; i<SERVERCOUNT;i++){
		if(i+1 == SELFNODEID){
			continue;
		}
		printf("%8d%10d%8d\n", i+1,NH[i],Dv[i]);
	}
	return 0;

}

int printServerList(){
	serverList_t *ptr = sHead;
	int i=1;
		while(ptr != NULL)
		{
			printf("%3d%26s%12d\n", i++, ptr->serverDetails.IPAddress, ptr->serverDetails.port);
			ptr = ptr->next;
		}
	return 0;
}

/*int printNeighbourList(){
	serverList_t *ptr = sHead;
		while(ptr != NULL)
		{
			printf("%3d%5d%5d\n", SELFNODEID, ptr->serverDetails.serverID, ptr->serverDetails.cost);
			ptr = ptr->next;
		}
	return 0;
}*/
int printNeighbourList(){
	neighbourList_t *ptr = nHead;
		while(ptr != NULL)
		{
			printf("%3d%5d%5d\n", SELFNODEID, ptr->neighbourDetails.neighbourID, ptr->neighbourDetails.cost);
			ptr = ptr->next;
		}
	return 0;
}

int addToNeighbourList(struct myNeighbourDetails neighbourStruct){
	neighbourList_t* tempPtr = (neighbourList_t*) malloc(sizeof(neighbourList_t));
	tempPtr->neighbourDetails.neighbourID=neighbourStruct.neighbourID;
	tempPtr->neighbourDetails.cost=neighbourStruct.cost;
	tempPtr->next = nHead;
	nHead = tempPtr;
	return 0;
}


int addNeighbourToServerList(int nodeID, int cost){
	serverList_t *current = sHead;
	if(sHead == NULL)
        {
                return -1;
        }
        while(current != NULL){
                if(current->serverDetails.serverID != nodeID){
                        current = current->next;
                }else{
			current->serverDetails.cost=cost;
			current->serverDetails.isNeighbour=true;
			current->serverDetails.linkStatus=ACTIVE;
			current->serverDetails.nextHop=nodeID;
			return 0;
		}
	}
                return -1;
}


int addToServerList(struct myServerDetails hostStruct){
	int i;
	serverList_t* tempPtr = (serverList_t*) malloc(sizeof(serverList_t));
	tempPtr->serverDetails.serverID=hostStruct.serverID;
	strncpy(tempPtr->serverDetails.IPAddress,hostStruct.IPAddress,strlen(hostStruct.IPAddress));
	tempPtr->serverDetails.cost=hostStruct.cost;
	tempPtr->serverDetails.port=hostStruct.port;
	tempPtr->serverDetails.isNeighbour=hostStruct.isNeighbour;
	tempPtr->serverDetails.expiryTime=hostStruct.expiryTime;
	tempPtr->serverDetails.linkStatus = ACTIVE;
	for(i=0;i<SERVERCOUNT;i++){
		tempPtr->serverDetails.neighbourDv[i]=INT_MAX;
	}
	tempPtr->next = sHead;
	sHead = tempPtr;
	return 0;
}

int isNeighbourCheck(int nodeID){
	serverList_t* current = sHead;
	if(sHead == NULL)
	{
		return NULL;
	}
	while(current != NULL){
		if(current->serverDetails.serverID != nodeID){
			current = current->next;
		}else{
			if(current->serverDetails.isNeighbour == true){
				return true;
			}else{
				return false;
			}
		}
	}
	return false;

}
