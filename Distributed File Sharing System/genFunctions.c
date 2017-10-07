#define _GNU_SOURCE
#include "genFunctions.h"
#include "registryList.h"
extern node_t* head_server;
extern node_t* head_client;
extern int maxFD;
extern int globPort;
extern fd_set readfds;
int globPort;
int run(int port, short int runMode){
	int n = 0, i=0, ret = 0, sockfd = 0, dataSockfd = 0;
	//struct sockaddr_in serverAddr, clientAddr;
	struct sockaddr_in clientAddr;
	fd_set tempreadfds;
	char buff[MAXBUFF+6];
	char buffToWrite[MAXBUFF];
	char send_buf[MAXBUFF+6];
	socklen_t cliLen=0;
	char command[CMDBUFF];
	struct addrinfo hints, *servinfo, *p;
//	struct sockaddr_storage their_addr;
	int rv, yes=1;
	char portString[6];
	char writeBuff[MAXBUFF+5];
	struct myIPPortStruct tempIPPortStruct;
	//struct in_addr ipv4addr;
	//struct hostent *he;
	char host[1024];
	char service[20];
	char cmdString[5];

	node_t *ptrAlreadyReg;
//	node_t *delptr;
	node_t* broadCastPtr;
	node_t* getFileName;
	char serializedCommand[512];
	int bytes_sent;
	int read_bytes;
	int readBytes;
	char fileToSend[32];
	long remain_data;
	int sent_bytes = 0;
	int len, fileTransferMode;
	char file_size[256];
	long fileSize;
	char destLoc[64]= "dest/";
	const char* destLocRoot = "dest/";
	char fileName[64];
	FILE *fd;
	off_t fsize;
	char *cmdCopy;
        char *putStr;
	int tc;

								globPort=port;
	if (runMode == SERVERMODE){
		printf("Server port is:%d\n", port);
	} else {
		printf("Client port is:%d\n", port);
	}

	FD_ZERO(&readfds);
	FD_ZERO(&tempreadfds);
	// beej code from here
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	sprintf(portString, "%d", port);
	rv = getaddrinfo(NULL, portString, &hints, &servinfo);
	if (rv !=0){
		printf("getaddrinfo: %s\n",gai_strerror(rv));
		return -1;
	}

	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}
		break;
	}
	//	freeaddrinfo(servinfo); // all done with this structure
	if (p == NULL) {
		printf("server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	printf("Listening on %s:%d..\n",inet_ntoa(((struct sockaddr_in *)(p->ai_addr))->sin_addr),((struct sockaddr_in *)(p->ai_addr))->sin_port);
	//beej code till here	
	/*	sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if(sockfd < 0){
		printf("Unable to create socket:%d\n",sockfd);
		return sockfd;
		}

		memset((void *)&serverAddr, 0, sizeof(serverAddr));
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.s_addr = INADDR_ANY;
		serverAddr.sin_port = htons(port);

		ret = bind(sockfd, (const struct sockaddr *) &serverAddr, sizeof(serverAddr));
		if(ret < 0){
		printf("Unable to bind to created socket Ret:%d\n",ret);
		return ret;
		}

		ret = listen(sockfd, maxConn);
		printf("listening on socket:%d, ret:%d\n",sockfd,ret);
		if(ret < 0){
		printf("Unable to listen on Port:%d Ret:%d\n",port,ret);
		return ret;
		}
		*/
	FD_SET(sockfd, &readfds);
	FD_SET(0, &readfds);

	maxFD = sockfd;
	cliLen = sizeof(clientAddr);

	while(1){
		tempreadfds = readfds;
		ret = select(maxFD+1, &tempreadfds, NULL, NULL, NULL);
		if (ret < 0) 
		{
			printf("select error\n");
			return ret;
		}
		if(FD_ISSET(0, &tempreadfds)){
			if(fgets(command, CMDBUFF, stdin)){
				ret = handleCMDs(command,*(struct sockaddr_in *)(p->ai_addr),runMode,sockfd);
			}
			//		        return ret;
		}
		for (i = 1 ; i <= maxFD ; i++){
			if(FD_ISSET(i, &tempreadfds)){
				if (i == sockfd){
					dataSockfd = accept(sockfd, (struct sockaddr *) &clientAddr, &cliLen);
					if (dataSockfd == -1){
						printf("accept error\n");
						//return ret;
						continue;
					} else {
						FD_SET(dataSockfd, &readfds);
						if (dataSockfd>maxFD){
							maxFD=dataSockfd;
						}
						printf("Bind received from IP:%s Port:%d Socket:%d DataPort:%d DataSocket:%d\n", inet_ntoa(clientAddr.sin_addr), port, sockfd, clientAddr.sin_port, dataSockfd);
					}
				} else {
					bzero(buff,MAXBUFF+6);
					if ((n = read(i,buff,MAXBUFF+5 ))<=0)
					{
						if (n == 0){
							printf("socket[%d] hung up\n",i);
							removeFromHostListSockfd(i,runMode);
							if (runMode == SERVERMODE){
								serializeList(UPDATE,serializedCommand,runMode);
								for (broadCastPtr=head_server;broadCastPtr != NULL; broadCastPtr=broadCastPtr->next){
									if((bytes_sent = send(broadCastPtr->hostAddr.sockfd, serializedCommand, strlen(serializedCommand), 0))<0){
										printf("Error when broadcasting Server IP List:%s\n",strerror(errno));
									}
									printf("Sending list:[%s] to %s at %d\n",serializedCommand,broadCastPtr->hostAddr.IPAddress, broadCastPtr->hostAddr.sockfd);
								}
							}
						}else{
							printf("socket[%d] some error:%d\n",i,n);
						}
						close(i);
						FD_CLR(i, &readfds);
					} else {
						memset(tempIPPortStruct.hostName,0,32);
						memset(tempIPPortStruct.IPAddress,0,16);
						strncpy(cmdString, buff, 4);
			//			if (atoi(cmdString) != GET_ACK){
							buff[n] = '\0';
			//			}
						if (atoi(cmdString) == REGISTER){
							if (runMode == SERVERMODE){
								printf("server received register command\n");
								memcpy(tempIPPortStruct.IPAddress,inet_ntoa(clientAddr.sin_addr),strlen(inet_ntoa(clientAddr.sin_addr)));
								//inet_pton(AF_INET, tempIPPortStruct.IPAddress, &ipv4addr);
								//he = gethostbyaddr(&ipv4addr, sizeof ipv4addr, AF_INET);
								getnameinfo((const struct sockaddr *)&clientAddr, sizeof clientAddr, host, sizeof host, service, sizeof service, 0);


								memcpy(tempIPPortStruct.hostName,host,strlen(host));
								if(((ptrAlreadyReg = findIPinList(tempIPPortStruct.IPAddress, SERVERMODE))!= NULL) || ((ptrAlreadyReg = findHostinList(tempIPPortStruct.hostName, SERVERMODE))!=NULL)){
									printf("Destination Client from %s(%s) already registered\n",tempIPPortStruct.IPAddress,tempIPPortStruct.hostName);
									close(i);
									continue;
								}
					//			tempIPPortStruct.port = ntohs(clientAddr.sin_port);
								tempIPPortStruct.port = atoi(buff+5);
								tempIPPortStruct.sockfd = i;
								tempIPPortStruct.runMode = CLIENTMODE ;
								addToHostList(tempIPPortStruct,runMode);
								serializeList(atoi(cmdString),serializedCommand,runMode);
								for (broadCastPtr=head_server;broadCastPtr != NULL; broadCastPtr=broadCastPtr->next){
									bytes_sent = send(broadCastPtr->hostAddr.sockfd, serializedCommand, strlen(serializedCommand), 0);
									printf("Sending list:[%s] to %s at %d\n",serializedCommand,broadCastPtr->hostAddr.IPAddress, broadCastPtr->hostAddr.sockfd);
								}
							}
							else{
								printf("I'am client. You cannot register to me. Closing connection..\n");
								//TODO close conn
								close(i);
								FD_CLR(i, &readfds);
							}
							printf("Data is:%s\n",buff);
						}else if (atoi(cmdString)== REGISTER_ACK){
							if (runMode == CLIENTMODE){
								printf("client received register_ack command and list to update\n");
								head_server = NULL;
							/*	tempIPPortStruct.port = ntohs(clientAddr.sin_port);
								tempIPPortStruct.sockfd = i;
								 memcpy(tempIPPortStruct.IPAddress,inet_ntoa(clientAddr.sin_addr),strlen(inet_ntoa(clientAddr.sin_addr)));
								getnameinfo((const struct sockaddr *)&clientAddr, sizeof clientAddr, host, sizeof host, service, sizeof service, 0);
								memcpy(tempIPPortStruct.hostName,host,strlen(host));
								tempIPPortStruct.runMode = SERVERMODE;
								addToHostList(tempIPPortStruct,runMode);
							*/	deSerialize(buff,1-runMode,i);
								printHostList(SERVERMODE);		
								//	deSerialize(buff,runMode,i);
							}
						}else if (atoi(cmdString) == CONNECT){
							if (lengthOfHostList(CLIENTMODE)>=CONNECTLIMIT){
								printf("Dest is already connected to %d clients. Check LIST\n",CONNECTLIMIT);
								close(i);
								FD_CLR(i, &readfds);
								continue;
							}
							if (runMode == SERVERMODE){
								printf("I'am server. You cannot connect to me. Closing connection..\n");
								close(i);
								FD_CLR(i, &readfds);
								continue;
							} else {
								printf("client received connect	command\n");
								memcpy(tempIPPortStruct.IPAddress,inet_ntoa(clientAddr.sin_addr),strlen(inet_ntoa(clientAddr.sin_addr)));
								getnameinfo((const struct sockaddr *)&clientAddr, sizeof clientAddr, host, sizeof host, service, sizeof service, 0);
								memcpy(tempIPPortStruct.hostName,host,strlen(host));
								/*								if(!(((ptrAlreadyReg = findIPinList(tempIPPortStruct.IPAddress, SERVERMODE))!= NULL) || ((ptrAlreadyReg = findHostinList(tempIPPortStruct.hostName, SERVERMODE))!=NULL))){
																printf("Destination Client from %s(%s) not registered to server, so can't connect\n",tempIPPortStruct.IPAddress,tempIPPortStruct.hostName);
								//	close(i);
								continue;
								}*/
								tempIPPortStruct.port =atoi(buff+5) ;
								tempIPPortStruct.sockfd = i;
								tempIPPortStruct.runMode = CLIENTMODE ;
								addToHostList(tempIPPortStruct,runMode);
								serializeList(atoi(cmdString),serializedCommand,runMode);
								bytes_sent = send(i, serializedCommand, strlen(serializedCommand), 0);
							}

						}else if (atoi(cmdString)== CONNECT_ACK){
							if (runMode == CLIENTMODE){
								printf("client received connect_ack command updating list\n");
                                                                cmdCopy = strdupa(buff+5);
                                                                tc = 0;
                                                                while((putStr = strsep(&cmdCopy,"|"))){
                                                                        if(tc == 0){
								tempIPPortStruct.port = atoi(putStr);
                                                                                tc++;
                                                                        }else if(tc == 1){
								memcpy(tempIPPortStruct.IPAddress,putStr,strlen(putStr));
                                                                                tc++;
                                                                        }else if(tc == 2){
								memcpy(tempIPPortStruct.hostName,putStr,strlen(putStr));
                                                                                tc++;
                                                                        }
                                                                }

							//	getnameinfo((const struct sockaddr *)&clientAddr, sizeof clientAddr, host, sizeof host, service, sizeof service, 0);
								tempIPPortStruct.sockfd = i;
								tempIPPortStruct.runMode = CLIENTMODE ;
								addToHostList(tempIPPortStruct,runMode);
								//	deSerialize(buff,runMode,i);
							}
						}else if (atoi(cmdString) == UPDATE_ACK){
							if (runMode == CLIENTMODE){
								printf("client received update command\n");
								head_server = NULL;
								deSerialize(buff,1-runMode,i);
								//getSelfIP(selfIP);
								//delptr = findIPinList(ipAddr, SERVERMODE);
								//removeFromHostListSockfd(delptr->hostAddr.sockfd,SERVERMODE);

							}else{

								printf("I'am server. You cannot update to me. Closing connection..\n");
								//TODO close conn
								close(i);
								FD_CLR(i, &readfds);
							}
							//							printf("Data is:%s\n",buff);

						}else if (atoi(cmdString) == PUT_ACK){
                                                                getFileName = find(i,CLIENTMODE);
                                                                strncpy(fileName,getFileName->hostAddr.fileName,strlen(getFileName->hostAddr.fileName));
								printf("Will send the file:%s\n",fileName);
                                                                fileTransferMode = FILETRANSFERINPROGRESS;
								fd = fopen(fileName, "rb");
								fseek(fd, 0L, SEEK_END);
                                                        remain_data = ftell(fd);
                                                        rewind(fd);

                                                                while(1){
                                                                        read_bytes = fread(send_buf,sizeof(char),MAXBUFF,fd);
                                                                        readBytes=read_bytes;
                                                                        sent_bytes=0;
                                                                        printf("read %d bytes of %ld\n",read_bytes,remain_data);
                                                                        while(read_bytes>0){
                                                                                snprintf(writeBuff,read_bytes+5,"%d|%s",PUT,send_buf+sent_bytes);
                                                                                sent_bytes = write(i,writeBuff,read_bytes+5);
                                                                        printf("sent %d bytes of %d\n",sent_bytes,read_bytes+5);
										//                                                                      if( (sent_bytes = send(i, writeBuff, strlen(writeBuff), 0)) < (read_bytes+5) ){

										//                              if( (sent_bytes = send(i, writeBuff, strlen(writeBuff), 0)) < (read_bytes) ){
										//                                      perror("send error");
										read_bytes=read_bytes-sent_bytes;
									}
									if (readBytes < MAXBUFF)
									{
										if (feof(fd))
											printf("sending complete\n");
										if (ferror(fd))
											printf("Error reading\n");
										sent_bytes=0;
									strcpy(getFileName->hostAddr.fileName,"");
										break;

									}
									}
								fclose(fd);
									fileTransferMode = FILETRANSFERNOTINPROGRESS;

						}else if (atoi(cmdString) == PUT){

							if(fileTransferMode!=FILETRANSFERINPROGRESS){
								cmdCopy = strdupa(buff+5);
								tc = 0;
								while((putStr = strsep(&cmdCopy,"|"))){
									if(tc == 0){
										remain_data = atol(putStr);
										tc++;
									}else if(tc == 1){
										strncpy(fileToSend,putStr,strlen(putStr));
										tc++;
									}
								}
								printf("File Size of %s is %ld bytes\n",fileToSend, remain_data);
								getFileName = find(i,CLIENTMODE);
								if (getFileName == NULL){
									printf("Connection No:%d doesn't exist or its not connected \n",i);
									continue;

								}
								strncpy(getFileName->hostAddr.fileName,fileToSend,strlen(fileToSend));
								strcpy(destLoc,destLocRoot);
								strncpy(fileToSend,getFileName->hostAddr.fileName,strlen(getFileName->hostAddr.fileName));
								strncat(destLoc,fileToSend,strlen(fileToSend));

										if( access( destLoc, F_OK ) != -1 ) {
											remove(destLoc);
											printf("File %s exists in destination location. Overwiting it.\n",destLoc);
										}

											printf("Waiting to recieve File %s into %s of size:%ld \n",fileToSend,destLoc,remain_data);
								sprintf(cmdString,"%d",PUT_ACK);//sent_bytes = write(i,(char *)PUT_ACK,4);
								sent_bytes = send(i,cmdString,4,0);
							if(sent_bytes<=0){printf("send error:%s\n", strerror(errno));}
							printf("Sending %s over %d\n",cmdString,i);

								fileTransferMode = FILETRANSFERINPROGRESS;
							}else{
								getFileName = find(i,CLIENTMODE);
								if(getFileName == NULL){
									printf("cant get filename from string");continue;
								}
printf("rcvbuffsize:%d\n",n);continue;
								strcpy(destLoc,destLocRoot);
								strncpy(fileName,getFileName->hostAddr.fileName,strlen(getFileName->hostAddr.fileName));
								strncat(destLoc,fileName,strlen(fileName));
								memset(buffToWrite,0,MAXBUFF);
								memcpy(buffToWrite,buff+5,n-5);
								//strcpy(buff,buff+5);
								fd = fopen(destLoc, "ab");
								printf("opened file %s to write\n",destLoc);
								//fwrite(buff, sizeof(char), n, received_file);
								fwrite(buffToWrite, sizeof(char), n-5, fd);
								remain_data=remain_data-(n-5);
								printf("Wrote %d bytes. Pending: %ld\n", n-5,remain_data);
								fclose(fd);

								if(n<sizeof(buff)-1){
									printf("Download complete last %d bytes copied\n",n-5);
									fileTransferMode = FILETRANSFERNOTINPROGRESS;
									//strcpy(getFileName->hostAddr.fileName,NULL);
									strcpy(getFileName->hostAddr.fileName,"");
									continue;
								}

							}

						}else if (atoi(cmdString) == GET){
							memcpy(fileToSend,buff+5,strlen(buff)-5);
							fd = fopen(fileToSend, "rb");
							if (fd == NULL)
							{
								printf("Error opening file --> %s", strerror(errno));

								exit(EXIT_FAILURE);
							}
							fseek(fd, 0L, SEEK_END);
							fsize = ftell(fd);
							rewind(fd);
							printf("File Size of %s is %ld bytes\n",fileToSend, (long)fsize);
							sprintf(file_size, "%d|%ld", GET_ACK,fsize);

							len = send(i, file_size, strlen(file_size), 0);
							if (len < 0)
							{
								printf("Error on sending greetings --> %s", strerror(errno));
								continue;
							}

							printf("Server sent %d bytes for the size of the file[%s]\n", len,fileToSend);

							remain_data = fsize;

							if (runMode == CLIENTMODE){
								fileTransferMode = FILETRANSFERINPROGRESS;
								while(1){
									read_bytes = fread(send_buf,sizeof(char),MAXBUFF,fd);
									readBytes=read_bytes;
									sent_bytes=0;
									printf("read %d bytes of %ld\n",read_bytes,remain_data);
									while(read_bytes>0){
										snprintf(writeBuff,read_bytes+5,"%d|%s",GET_ACK,send_buf+sent_bytes);
										//sent_bytes = write(i,writeBuff,read_bytes+5);
										sent_bytes = send(i,writeBuff,read_bytes+5,0);
printf("last 10 char:[%s]\n",writeBuff+(sent_bytes-10));
									printf("sent %d bytes of %d\n",sent_bytes,read_bytes);
										//									if( (sent_bytes = send(i, writeBuff, strlen(writeBuff), 0)) < (read_bytes+5) ){

										//				if( (sent_bytes = send(i, writeBuff, strlen(writeBuff), 0)) < (read_bytes) ){
										//					perror("send error");
										read_bytes=read_bytes-sent_bytes;
									}
									if (readBytes < MAXBUFF)
									{
										if (feof(fd))
											printf("sending complete\n");
										if (ferror(fd))
											printf("Error reading\n");
										sent_bytes=0;
										break;

									}
									}
								fileTransferMode = FILETRANSFERNOTINPROGRESS;
							}else{

								printf("I'am server. You cannot download from server..\n");
								//TODO close conn
							}                                                
								fclose(fd);
								}else if (atoi(cmdString) == GET_ACK){
						//buff[n-1] = '\0';
									getFileName = find(i,CLIENTMODE);
									strcpy(destLoc,destLocRoot);
									strncpy(fileName,getFileName->hostAddr.fileName,strlen(getFileName->hostAddr.fileName));
									strncat(destLoc,fileName,strlen(fileName));
									if(fileTransferMode == FILETRANSFERINPROGRESS){
										memcpy(buffToWrite,buff+5,n-5);
										//strcpy(buff,buff+5);
										fd = fopen(destLoc, "ab");
										printf("will download file as [%s]\n",destLoc);
										//fwrite(buff, sizeof(char), n, received_file);
										fwrite(buffToWrite, sizeof(char), n-5, fd);
printf("last 10 char:[%s]\n",buffToWrite+(n-10));
										remain_data=remain_data-(n-5);
										printf("Wrote %d bytes. Pending: %ld\n", n-5,remain_data);
										fclose(fd);
										if(n<sizeof(buff)-1){
											//	if(remain_data <= 0){
											printf("Download complete last %d bytes copied\n",n-5);
											fileTransferMode = FILETRANSFERNOTINPROGRESS;
											//strcpy(getFileName->hostAddr.fileName,NULL);
											strcpy(getFileName->hostAddr.fileName,"");
										}
									}else{
										fileSize = atol(buff+5);
										if( access( destLoc, F_OK ) != -1 ) {
											remove(destLoc);
											printf("File %s exists in destination location. Overwiting it.\n",destLoc);
										}

										printf("Receiving file of size:%ld into %s\n",fileSize,destLoc);
										remain_data = (long)fileSize;
										fileTransferMode = FILETRANSFERINPROGRESS;

									}
										//							printf("Data is:%s\n",buff);
									}
									else{printf("Invalid Command:%s\n",buff);}

								}

				}
			}


		}


	}
	return ret;
}


int updateRegistry(struct sockaddr_in clientAddr){
	return 0;
}

int printHelp(){
	printf("COMMANDS:\n==========\n\n ");
	printf("1) HELP: Displays available user commands\n 2) CREATOR: Displays student's full name, UBIT name, UB email address\n 3) DISPLAY: Diplays IP address and the listening port of this process\n 4) REGISTER <server_ip> <port>: Used to register the client with the server and to get info of other clients. Can't use this when running in server mode\n 5) CONNECT <destination> <port>: Used to establish connection with other clients. Can't use this when running in server mode\n ");
	printf("6) LIST: Displays all connections with this processs. Can't use this when running in server mode\n 7) TERMINATE <connection_id>: Used to terminate a connection displayed from the LIST command of the same process\n 8) QUIT: Close all connections and terminate this process\n 9) GET <connection_id> <file>: Used to download a file from the connection specified in the command\n 10) PUT <connection_id> <file>: Used to upload a file to the connection specified in the command\n");
	return 0;
}

int printCreator(){
	printf("Name: Prashanth Chandrashekar\nUBIT: pc74\nUBIT Email Address: pc74@buffalo.edu\nPerson Number: 50207911\n");
	return 0;
}

int handleCMDs(char* command, struct sockaddr_in serverAddr, short int mode, int sockfd){
	char *cmdCopy;
	char *token[5];
	char* pEnd;
	char selfIP[16];
	char selfHost[32];
	int i=0, port=0, noOfTokens=0;
	node_t *ptrAlreadyReg;

	cmdCopy = strdupa(command);

	//	printf("incoming text: %s\n",command);

	cmdCopy[strcspn(cmdCopy, "\r\n")] = 0;
	while((token[i] = strsep(&cmdCopy," "))){
		if(i >= 4){
			printf("Invalid input. Too many parameters\n");
			return -1;
		}
		i++;
	}
	noOfTokens=i;
	if((strcasecmp(token[0],"help")==0)){
		if(noOfTokens == 1){
			printHelp();
		}
		else{
			printf("Invalid number of inputs for %s\n",token[0]);
		}
	} else if((strcasecmp(token[0],"creator")==0)){
		if(noOfTokens == 1){
			printCreator();
		}
		else{
			printf("Invalid number of inputs for %s\n",token[0]);
		}
	} else if((strcasecmp(token[0],"display")==0)){
		if(noOfTokens == 1){
			getSelfIP(selfIP);
			printf("Process is running on the server: %s and is listening on the port:%d\n",selfIP,ntohs(serverAddr.sin_port));
		}
		else{
			printf("Invalid number of inputs for %s\n",token[0]);
		}
	} else if((strcasecmp(token[0],"register")==0)){
		if(mode == CLIENTMODE){
			if(noOfTokens == 3){
				port = strtol(token[2],&pEnd,10);
				getSelfIP(selfIP);
				if((ptrAlreadyReg = findIPinList(selfIP, SERVERMODE))!= NULL){
					printf("This Client is already registered to server\n");
					//      close(i);
					return 0;
				}
				registerOrConnect(ntohs(serverAddr.sin_port),token[1],port,REGISTER);
			}
			else{
				printf("Invalid number of inputs for %s\n",token[0]);
			}
		} else {
			printf("REGISTER can be used only in Client mode\n");
		}
	} else if((strcasecmp(token[0],"connect")==0)){
		if(mode == CLIENTMODE){
			getSelfIP(selfIP);
			getSelfHost(selfHost);
			if(strncmp(token[1],selfIP,strlen(token[1]))==0){
				printf("Trying to connect to same server!!\n");
				return 0;
			}
			if(strncmp(token[1],selfHost,strlen(token[1]))==0){
				printf("Trying to connect to same server!!\n");
				return 0;
			}
			if(noOfTokens == 3){
				port = strtol(token[2],&pEnd,10);

				if((ptrAlreadyReg = findIPinList(selfIP, SERVERMODE))== NULL){
					printf("This Client is not registered to server, so can't connect\n");
					//      close(i);
					return 0;
				}

				registerOrConnect(ntohs(serverAddr.sin_port),token[1],port,CONNECT);
				//connectToPeerClient(sockfd,token[1],port);
			}
			else{
				printf("Invalid number of inputs for %s\n",token[0]);
			}
		} else {
			printf("CONNECT can be used only in Client mode\n");
		}
	} else if((strcasecmp(token[0],"list")==0)){
		if(noOfTokens == 1){
			printHostList(mode);
			//printHostList(1-mode);
		}
		else{
			printf("Invalid number of inputs for %s\n",token[0]);
		}
	} else if((strcasecmp(token[0],"terminate")==0)){
		if(noOfTokens == 2){
			terminate(strtol(token[1],&pEnd,10),mode);
		}
		else{
			printf("Invalid number of inputs for %s\n",token[0]);
		}
	} else if((strcasecmp(token[0],"quit")==0)){
		if(noOfTokens == 1){
			printf("Closing all connections and Going down..\n");
			exit(0);
		}
		else{
			printf("Invalid number of inputs for %s\n",token[0]);
		}
	} else if((strcasecmp(token[0],"get")==0)){
		if(mode == CLIENTMODE){
			if(noOfTokens == 3){
				upDownFile(strtol(token[1],&pEnd,10),token[2], GET,"get");
			}
			else{
				printf("Invalid number of inputs for %s\n",token[0]);
			}
		} else {
			printf("GET can be used only in Client mode\n");
		}
	} else if((strcasecmp(token[0],"put")==0)){
		if(mode == CLIENTMODE){
			if(noOfTokens == 3){
				upDownFile(strtol(token[1],&pEnd,10),token[2], PUT,"put");
			}
			else{
				printf("Invalid number of inputs for %s\n",token[0]);
			}
		} else {
			printf("PUT can be used only in Client mode\n");
		}
	} else {
		printf("Invalid INPUT. Check HELP command.\n");
	}

	free(cmdCopy);
	return 0;
}

int getSelfHost(char* selfIP){
	char hostname[1024];
	//struct hostent *he;
	//struct in_addr **addr_list;
	hostname[1023] = '\0';
	gethostname(hostname, 1023);
	strncpy(selfIP,hostname,strlen(hostname));
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

int registerOrConnect(int listenPort,char* ipAddr,int port, int cmd){
	struct addrinfo hints, *res;
	int connectSockfd;
	char portString[6];
	char *msg = malloc(64);
	int len, bytes_sent;
	node_t *ptr;
	snprintf(msg, 64, "%d|%d", cmd,listenPort);
	len = strlen(msg);
	//struct sockaddr_in conAddr;
	// first, load up address structs with getaddrinfo():
	if(cmd == CONNECT){
		if (lengthOfHostList(CLIENTMODE)>=CONNECTLIMIT){
			printf("I am already connected to 3 clients. Check LIST\n");
			free(msg);
			return -1;
		}
		if((ptr = findIPinList(ipAddr, SERVERMODE))==NULL){
			if((ptr = findHostinList(ipAddr, SERVERMODE))==NULL){
				printf("Destination Client %s not registered\n",ipAddr);
				free(msg);
				return -1;
			}
		}
		if(((ptr = findIPinList(ipAddr, CLIENTMODE))!=NULL) || ((ptr = findHostinList(ipAddr, CLIENTMODE))!=NULL)){
			printf("Destination Client is already connected\n");
			free(msg);
			return -1;

		}
		if(ptr != NULL){
			if (ptr->hostAddr.runMode == SERVERMODE){
				printf("Destination is the server, please connect to other registered clients\n");
				free(msg);
				return -1;

			}
		}
	}
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	sprintf(portString, "%d", port);
	//	sprintf(portString, "%d", listenPort);

	getaddrinfo(ipAddr, portString, &hints, &res);
	// make a socket:
	connectSockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	/*	memset((void *)&conAddr, 0, sizeof(conAddr));
		conAddr.sin_family = AF_INET;
		conAddr.sin_addr.s_addr = INADDR_ANY;
		conAddr.sin_port = htons(port);*/
	// connect!
	//	res->ai_addr.sin_port = htons(port);
	if ((connect(connectSockfd, res->ai_addr, res->ai_addrlen))!=-1){
		printf("will connect to %s with %d, new socket: %d, sent command: %s\n",ipAddr,port,connectSockfd,msg);	
		if((bytes_sent = send(connectSockfd, msg, len, 0))<0){
			printf("Connect failed Err:%s\n",strerror(errno));
		}
		FD_SET(connectSockfd, &readfds);
		if (connectSockfd>maxFD){
			maxFD=connectSockfd;
		}

	}else{
		printf("Connect failed buddy! Problem with the port Chosen\n");
	}

	return 0;
}


int terminate(int conID, int mode){
	char serializedCommand[512];
	node_t* broadCastPtr;
	int bytes_sent,sockfd;
	sockfd = removeFromHostList(conID,mode);
	if (sockfd>0){
		close(sockfd);
                FD_CLR(sockfd, &readfds);
	printf("Closed the sockets and terminated: %d\n",conID);

	}
	if (mode == SERVERMODE){
		serializeList(UPDATE,serializedCommand,mode);
		for (broadCastPtr=head_server;broadCastPtr != NULL; broadCastPtr=broadCastPtr->next){
			if((bytes_sent = send(broadCastPtr->hostAddr.sockfd, serializedCommand, strlen(serializedCommand), 0))<0){
										printf("Error when broadcasting Server IP List:%s\n",strerror(errno));
			}
			printf("Sending list:[%s] to %s at %d\n",serializedCommand,broadCastPtr->hostAddr.IPAddress, broadCastPtr->hostAddr.sockfd);
		}		
	}
	return 0;
}

int upDownFile(int conID, char* fileName, int cmd, char * getPutCmd) {
	node_t *ptrAlreadyReg;
	char *msg = malloc(64);
	int bytes_sent;
	char selfIP[16];
	char selfHost[32];
	node_t *ptr;
	int sockfd,fsize;
	FILE* fd;
	getSelfIP(selfIP);
	getSelfHost(selfHost);
	if((ptrAlreadyReg = findIPinList(selfIP, SERVERMODE))== NULL){
		printf("This Client is not registered to server, so can't %s the file",getPutCmd);
		return 0;
	}

	ptr = find(conID,CLIENTMODE);
	if (ptr == NULL){
		printf("Connection No:%d doesn't exist or its not connected \n",conID);
		free(msg);
		return -1;

	}
	sockfd = ptr->hostAddr.sockfd;
	if(cmd==PUT){
							fd = fopen(fileName, "rb");
                                                        if (fd == NULL)
                                                        {
                                                                printf("Error opening file --> %s", strerror(errno));

		free(msg);
		return -1;
                                                        }
                                                        fseek(fd, 0L, SEEK_END);
                                                        fsize = ftell(fd);
                                                        rewind(fd);
	snprintf(msg, 64, "%d|%d|%s",cmd,fsize,fileName);
	}else{
	snprintf(msg, 64, "%d|%s",cmd,fileName);
	}
	strncpy(ptr->hostAddr.fileName,fileName,strlen(fileName));
	bytes_sent = send(sockfd, msg, strlen(msg), 0);
	if (bytes_sent<0){
		printf("Unable to send %s command\n",getPutCmd);
		free(msg);
		return -1;

	}

printf("Sent %s command as %s to Connection %d\n",getPutCmd,msg,sockfd);
		free(msg);
	return 0;
}

int downloadFile(int conID, char* fileName, int sockfd) {
	node_t *ptrAlreadyReg;
	char *msg = malloc(64);
	int bytes_sent;
	char selfIP[16];
	char selfHost[32];
	node_t *ptr;
	snprintf(msg, 64, "%d|%s",GET,fileName);
	getSelfIP(selfIP);
	getSelfHost(selfHost);
	if((ptrAlreadyReg = findIPinList(selfIP, SERVERMODE))== NULL){
		printf("This Client is not registered to server, so can't download\n");
		return 0;
	}

	ptr = find(conID,CLIENTMODE);
	if (ptr == NULL){
		printf("Source %d connection doesn't exist or its not connected \n",conID);
		free(msg);
		return -1;

	}
	sockfd = ptr->hostAddr.sockfd;
	strncpy(ptr->hostAddr.fileName,fileName,strlen(fileName));
	bytes_sent = send(sockfd, msg, strlen(msg), 0);
	if (bytes_sent<0){
		printf("Unable to send download command\n");
		free(msg);
		return -1;

	}


	return 0;
}


int deSerialize(char* buff, int mode, int cliSock){
	char *cmdCopy;
	char *token;
	//int i=0, port=0, noOfTokens=0, len=0;
	int i=0, len=0;
	struct myIPPortStruct tempIPPortStruct;
	//	char selfIP[16];

	cmdCopy = strdupa(buff);

	//		getSelfIP(selfIP);
	while((token = strsep(&cmdCopy,"|"))){
		if(i == 0){
			i++;
			continue;
		}else if(i == 1){
			i++;
			len = atoi(token);
			continue;
		}
		tempIPPortStruct = (struct myIPPortStruct)formListStruct(token, mode, cliSock);
		//		if((strcmp(selfIP,tempIPPortStruct.IPAddress))==0){
		//			continue;
		//		}
		//memcpy(tempIPPortStruct,formListStruct(token),sizeof(formListStruct(token)));
		addToHostList(tempIPPortStruct, mode);
		if(len-- == 0){
			break;
		}

	}
	return 0;

}

struct myIPPortStruct formListStruct(char* buff, int mode, int cliSock){
	char *cmdCopy;
	char *token[5];
	int i=0;// port=0, noOfTokens=0, len=0;
	struct myIPPortStruct tempIPPortStruct;

	cmdCopy = strdupa(buff);

	while((token[i++] = strsep(&cmdCopy,":")));
	strcpy(tempIPPortStruct.IPAddress,token[0]);
	strcpy(tempIPPortStruct.hostName,token[1]);
	tempIPPortStruct.port = atoi(token[2]);
	if(mode == SERVERMODE){
		tempIPPortStruct.sockfd = atoi(token[3]);
	}else{
		tempIPPortStruct.sockfd = cliSock;
	}
	tempIPPortStruct.runMode = atoi(token[4]);

	return tempIPPortStruct; 

}

int serializeList(int cmdString, char *buff, int mode){
	int i=0, listLen=0;
	node_t* ptr=head_server;
	node_t* ptr1=head_client;
	        char selfIP[16];
        char selfHost[32];


if(cmdString==REGISTER){	listLen=lengthOfHostList(mode);

	sprintf(buff,"%d|%d",cmdString+1000,listLen);
	while(i<listLen){
		if (mode == SERVERMODE && ptr != NULL){
			sprintf(buff+strlen(buff),"|%s:%s:%d:%d:%d",ptr->hostAddr.IPAddress,ptr->hostAddr.hostName,ptr->hostAddr.port,ptr->hostAddr.sockfd,ptr->hostAddr.runMode);
			ptr= ptr->next;
			i++;
		} else if (mode == CLIENTMODE && ptr1 != NULL){
			sprintf(buff+strlen(buff),"|%s:%s:%d:%d:%d",ptr->hostAddr.IPAddress,ptr->hostAddr.hostName,ptr->hostAddr.port,ptr->hostAddr.sockfd,ptr->hostAddr.runMode);
			ptr1= ptr1->next;
			i++;
		}
	}}
else if (cmdString==CONNECT){
        getSelfIP(selfIP);
        getSelfHost(selfHost);
	sprintf(buff,"%d|%d|%s|%s",cmdString+1000,globPort,selfIP,selfHost);
}
	return 0;
}
