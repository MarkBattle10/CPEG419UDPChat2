#include <stdio.h> /*for printf() and fprintf()*/
#include <sys/socket.h> /*for socket(), connect(), send(), and recv()*/
#include <arpa/inet.h> /*for sockaddr_in and inet_addr()*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NAMEMAX 255 /*Longest string to echo*/
#define MAXUSERS 3
#define MAXMSG 1024

void DieWithError(char *errorMessage);

int main(int argc, char *argv[])
{
	int sock; /*Socket descriptor*/
	struct sockaddr_in echoServAddr; /* Echo server address */
	struct sockaddr_in fromAddr; /* Source address of echo */
	unsigned short echoServPort;
	unsigned int fromSize;
	char *servIP;
	char *userName;
	char echoBuffer[NAMEMAX+1];
	char usersBuffer[MAXUSERS][NAMEMAX];
	int numUsers;
	int cliExists;
	unsigned int userNameLen;
	int respStringLen;
	char sendMsgBuf[MAXMSG];
	char recvMsgBuf[MAXMSG];

	if ((argc<3)||(argc>4))
	{
			fprintf(stderr, "Usage: %s <Server IP> <Username> [<Echo Port>]\n"), argv[0];
			exit(1);
	}

	servIP = argv[1];
	userName = argv[2];
	
	if ((userNameLen = strlen(userName)) > NAMEMAX) /* Check input length */
		DieWithError("Username too long");

	if(argc == 4)
		echoServPort = atoi(argv[3]);
	else
		echoServPort = 7;

	/* Create a datagram/UDP socket */
	if((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP))<0)
		DieWithError("socket() failed");

	/*Construct the server address structure*/
	memset(&echoServAddr, 0, sizeof(echoServAddr)); /* Zero out structure */
	echoServAddr.sin_family = AF_INET;   /* Internet addr family */
	echoServAddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
	echoServAddr.sin_port = htons(echoServPort);  /* Server port */

	
	/*send the string to the server*/
	if (sendto(sock, userName, userNameLen, 0, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr))!=userNameLen)
		DieWithError("send() sent a different number of bytes than expected");

	fromSize = sizeof(fromAddr);
	numUsers = 0;
	for(int j = 0;j<MAXUSERS;j++){
		usersBuffer[j][0] = 0;
	}
	printf("Current Users: \n");
	for(;;){
		/*receive the usernames of the users in the chat*/
		if(numUsers<MAXUSERS){
			int i=0;
			cliExists=0;
			if((respStringLen = recvfrom(sock, echoBuffer, NAMEMAX, 0, (struct sockaddr *) &fromAddr, &fromSize)) < 0){
				DieWithError("recv() failed or connection closed prematurely");
			}
			echoBuffer[respStringLen] = '\0';
				
				for(int j=0;j<=numUsers;j++){
					if(strcmp(usersBuffer[j],echoBuffer) == 0){
						cliExists = 1;
					}
				}
			
			if(!cliExists){
				strcpy(usersBuffer[numUsers], echoBuffer);
				printf("%s\n", usersBuffer[numUsers]);
				numUsers++;
				//printf("numUsers: %d\n",numUsers);
			}
		}
		/*after all users enter the chatroom, begin sending messages*/
		else{	
			int charIter = 0;
			while((sendMsgBuf[charIter++] = getchar())!= '\n');
			printf("%s\n", sendMsgBuf);
			if (sendto(sock, sendMsgBuf, strlen(sendMsgBuf), 0, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr))<0)
				DieWithError("send() sent a different number of bytes than expected");
			if((respStringLen = recvfrom(sock, recvMsgBuf, MAXMSG, 0, (struct sockaddr *) &fromAddr, &fromSize)) < 0){
				DieWithError("recv() failed or connection closed prematurely");
			}

		}


	}



	/*Receive a response*/
	/*
	fromSize = sizeof(fromAddr);
	if ((respStringLen = recvfrom(sock, echoBuffer, NAMEMAX, 0, (struct sockaddr *) &fromAddr, &fromSize)) != userNameLen)
		DieWithError("recvfrom() failed") ;
	*/

	if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
	{
		fprintf(stderr,"Error: received a packet from unknown source.\n");
		exit(1);
	}
	
	
	/* null-terminate the received data */
//	echoBuffer[respStringLen] = '\0' ;
//	printf("Current Users: %s\n", echoBuffer); /* Print the echoed arg */
	close(sock);
	exit(0);
}
