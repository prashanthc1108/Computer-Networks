#include "registryList.h"
#include "genFunctions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern node_t* head_server;
extern node_t* head_client;

int printHostList(int mode){
	node_t *ptr = head_server;
	node_t *ptr1 = head_client;
	int i=1;
	if (mode == SERVERMODE){
		printf("Clients Registered with Server(Server-IP-List):\n============================================\n");
		printf("ID:              Hostname:                      IP Address:        Port No:\n===              =========                      ===========        ========\n");
		while(ptr != NULL)
		{
//			printf("%3d%32s%26s%12d\n", i++, ptr->hostAddr.hostName, ptr->hostAddr.IPAddress, ptr->hostAddr.port);
			printf("%3d%32s%26s%12d\n", i++, ptr->hostAddr.hostName, ptr->hostAddr.IPAddress, ptr->hostAddr.port);
			ptr = ptr->next;
		}
	} else if (mode == CLIENTMODE) {
		printf("Connected Peers:\n================\n");
		printf("ID:              Hostname:                      IP Address:        Port No:\n===              =========                      ===========        ========\n");
		while(ptr1 != NULL)
		{
//			printf("%3d%32s%26s%12d\n", i++, ptr1->hostAddr.hostName, ptr1->hostAddr.IPAddress, ptr1->hostAddr.port);
			printf("%3d%32s%26s%12d\n", i++, ptr1->hostAddr.hostName, ptr1->hostAddr.IPAddress, ptr1->hostAddr.port);
			ptr1 = ptr1->next;
		}
	}
	return 0;
}

int addToHostList(struct myIPPortStruct hostStruct, int mode){
	node_t* tempPtr = (node_t*) malloc(sizeof(node_t));
	memcpy(tempPtr->hostAddr.IPAddress,hostStruct.IPAddress,strlen(hostStruct.IPAddress));
	memcpy(tempPtr->hostAddr.hostName,hostStruct.hostName,strlen(hostStruct.hostName));
	tempPtr->hostAddr.port = hostStruct.port;
	tempPtr->hostAddr.sockfd = hostStruct.sockfd;
	tempPtr->hostAddr.runMode = hostStruct.runMode;
	if (mode == SERVERMODE){
		tempPtr->next = head_server;
		head_server = tempPtr;
	} else if (mode == CLIENTMODE) {
		tempPtr->next = head_client;
		head_client = tempPtr;
	}
	return 0;
}

bool isHostListEmpty(int mode)
{
	if (mode == SERVERMODE){
		return head_server == NULL;
	} else {
		return head_client == NULL;
	}
}

int lengthOfHostList(int mode)
{
	int length = 0;
	struct node *current;

	if (mode == SERVERMODE){
		for(current = head_server; current != NULL; current = current->next)
		{
			length++;
		}
	} else if (mode == CLIENTMODE) {
		for(current = head_client; current != NULL; current = current->next)
		{
			length++;
		}
	}	
	return length;
}

struct node* findIPinList(char* key, int mode){
	struct node* current = head_server;
	struct node* current1 = head_client;
	if (mode == SERVERMODE){
		if(head_server == NULL)
		{
			return NULL;
		}
		while((current != NULL) && (strncmp(current->hostAddr.IPAddress,key,strlen(key)) != 0)){
			current = current->next;
		}
		return current;
	} else  {
		if(head_client == NULL)
		{
			return NULL;
		}
		while((current1 != NULL) && (strncmp(current1->hostAddr.IPAddress,key,strlen(key)) != 0)){
			current1 = current1->next;
		}
		return current1;
	}
}


struct node* findHostinList(char* key, int mode){
	struct node* current = head_server;
	struct node* current1 = head_client;
	if (mode == SERVERMODE){
		if(head_server == NULL)
		{
			return NULL;
		}
		while((current != NULL) && (strncmp(current->hostAddr.hostName,key,strlen(key)) != 0)){
			current = current->next;
		}
		return current;
	} else  {
		if(head_client == NULL)
		{
			return NULL;
		}
		while((current1 != NULL) && (strncmp(current1->hostAddr.hostName,key,strlen(key)) != 0)){
			current1 = current1->next;
		}
		return current1;
	}
}


struct node* find(int key, int mode){
	struct node* current = head_server;
	struct node* current1 = head_client;
	int i=0;
	if (mode == SERVERMODE){
		if(head_server == NULL)
		{
			return NULL;
		}
		while((current->next != NULL) && (i<key)){
			current = current->next;
			i++;
		}      
		return current;
	} else  {
		if(head_client == NULL)
		{
			return NULL;
		}
		while((current1->next != NULL) && (i<key)){
			current1 = current1->next;
			i++;
		}
		return current1;
	}
}

int removeFromHostListSockfd(int sockfd, int mode){
	node_t *tempPtr = head_server;
	node_t *tempPtr1 = head_client;
	node_t *prevPtr = NULL;
	int i=1;
	if (mode == SERVERMODE){
		if(tempPtr == NULL){
			return -1;
		}
		while(tempPtr->hostAddr.sockfd!=sockfd){
			prevPtr = tempPtr;
			tempPtr = tempPtr->next;
			if(tempPtr == NULL){
				return -1;
			}
			i++;
		}
		if (tempPtr == head_server){
			head_server = head_server->next;
		} else {
			prevPtr->next = tempPtr->next;
		}
	} else if (mode == CLIENTMODE) {
		if(tempPtr1 == NULL){
			return -1;
		}
		while(tempPtr1->hostAddr.sockfd!=sockfd){
			prevPtr = tempPtr1;
			tempPtr1 = tempPtr1->next;
			if(tempPtr1 == NULL){
				return -1;
			}
			i++;
		}
		if (tempPtr1 == head_client){
			head_client = head_client->next;
		} else {
			prevPtr->next = tempPtr1->next;
		}
	}
	return 0;
}

int removeFromHostList(int key, int mode){
	node_t *tempPtr = head_server;
	node_t *tempPtr1 = head_client;
	node_t *prevPtr = NULL;
	int i=1, sockfd;
	if(key>lengthOfHostList(mode)){
		return -1;
	}
	if (mode == SERVERMODE){
		while(i<key){
			prevPtr = tempPtr;
			tempPtr = tempPtr->next;
			if(tempPtr == NULL){
				return -1;
			}
			i++;
		}
		sockfd= tempPtr->hostAddr.sockfd;
		if (tempPtr == head_server){
			head_server = head_server->next;
		} else {
			prevPtr->next = tempPtr->next;
		}
			return sockfd;
	} else if (mode == CLIENTMODE) {
		while(i<key){
			prevPtr = tempPtr1;
			tempPtr1 = tempPtr1->next;
			if(tempPtr1 == NULL){
				return -1;
			}
			i++;
		}
		sockfd= tempPtr1->hostAddr.sockfd;
		if (tempPtr1 == head_client){
			head_client = head_client->next;
		} else {
			prevPtr->next = tempPtr1->next;
		}
			return sockfd;
	}
	return -1;
}
