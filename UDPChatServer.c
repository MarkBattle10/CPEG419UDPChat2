#include <stdio.h> /*for printf() and fprintf()*/
#include <sys/socket.h> /*for socket(), connect(), send(), and recv()*/
#include <arpa/inet.h> /*for sockaddr_in and inet_addr()*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NAMEMAX 32   /* Longest string for username */
#define MAXUSERS 3
#define MSGSIZE 1024 /* size of message that can be sent */
#define TRUE 1
#define FALSE 0

void DieWithError(char *errorMessage); /* Error handling function */

typedef struct client{
	struct sockaddr_in clntAddr; /*client address*/
	char username[NAMEMAX]; /* hold clients username initialized with first connection */
} client_t;

int clientCompare(struct sockaddr_in client1, struct sockaddr_in client2){
	if(strcmp((char *) &client1.sin_addr.s_addr, (char *) &client2.sin_addr.s_addr) == 0){
		if(strcmp((char *) &client1.sin_port, (char *) &client2.sin_port) == 0){
			if(strcmp((char *) &client1.sin_family, (char *) &client2.sin_family) == 0){

				return TRUE;
			}
		}
	}
	return FALSE;
}


/*
void broadcast(struct sockaddr_in sender, int all, client_t *clientList){
	client_t *iter = clientList->next; 
	while(iter != NULL){
		
		iter = iter->next;
	}

}
*/

int main(int argc, char *argv[])
{

	int sock; /* Socket */
	struct sockaddr_in echoServAddr; /* Local address */
	struct sockaddr_in echoClntAddr;
	client_t clientList; /* to keep track of the clients */
	client_t *iter;
	unsigned int cliAddrLen; /* Length of incoming message */
	char nameBuffer[NAMEMAX]; /* Buffer for usernames */
	char userBuffer[MAXUSERS][NAMEMAX]; /*buffer to collect each name*/
	char requestMsgBuf[MSGSIZE];
	char sendMsgBuf[MSGSIZE];
	struct sockaddr_in strUserList[MAXUSERS];
	client_t clntList[MAXUSERS];
	int clientExists;
	int recvMsgBuf[MAXUSERS];
	int numUsers;
	//char numUsersChar;
	int removeEnds;
	unsigned short echoServPort; /* Server port */
	int recvMsgSize; /* Size of received message */

	if (argc != 2) /* Test for correct number of arguments */
	{
		fprintf(stderr, "Usage: %s <Server Port>\n", argv[0]) ;
		exit(1);
	}

	echoServPort = atoi(argv[1]); /* First arg: local port */

	/* Create socket for incoming connections */
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		DieWithError( "socket () failed") ;

	/* Construct local address structure */
	memset(&echoServAddr, 0, sizeof(echoServAddr)); /* Zero out structure */
	echoServAddr.sin_family = AF_INET; /* Internet address family */
	echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
	echoServAddr.sin_port = htons(echoServPort); /* Local port */

	/* Bind to the local address */
	if (bind(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
		DieWithError ( "bind () failed");


	numUsers = 0;
	for (;;) /* Run forever */
	{
		bzero(sendMsgBuf, MSGSIZE);
		cliAddrLen = sizeof(echoClntAddr);
		if(numUsers<MAXUSERS){
			//printf("made it here first");
			if((recvMsgSize = recvfrom(sock, nameBuffer, NAMEMAX, 0, (struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0){
				DieWithError("recvfrom() failed");
			}
			nameBuffer[recvMsgSize] = '\0';
			if(numUsers == 0){
				clntList[0].clntAddr = echoClntAddr;
				strcpy(clntList[0].username, nameBuffer);
				//numUsers++;
				printf("clntAddr: %d, username: %s\n", clntList[0].clntAddr.sin_port, clntList[0].username);

				if(sendto(sock, clntList[0].username, NAMEMAX, 0, &clntList[0].clntAddr, sizeof(clntList[0].clntAddr)) < 0){
					DieWithError("sendto() failed");
				}
			}
			int i = 0;
			while(i < numUsers){
				clientExists = FALSE;
				for(int h=0; h<=numUsers;h++){
					if(clientCompare(clntList[h].clntAddr, echoClntAddr) == TRUE){
						clientExists=TRUE;
					}
				}
				if(clientExists==TRUE){
					strcpy(sendMsgBuf, "already have this client");
					//if(sendto(sock, sendMsgBuf, MSGSIZE, 0, &echoClntAddr, sizeof(echoClntAddr)) < 0){
					//	DieWithError("sendto() failed");
					//}	
					
				}
				else{
					clntList[numUsers].clntAddr = echoClntAddr;
					strcpy(clntList[numUsers].username, nameBuffer);
					printf("new user added: %s\n", clntList[numUsers].username);
					for(int j = 0;j<=numUsers;j++){
						for(int k = 0; k<=numUsers; k++){
							if(sendto(sock, clntList[k].username, NAMEMAX, 0, &clntList[j].clntAddr, sizeof(clntList[j].clntAddr)) < 0){
								DieWithError("sendto() failed");
							}
						}
					}
					//numUsers++;

				}	
				i++;
					
			}
			numUsers++;	
			//printf("numUsers: %d, MAXUSERS: %d\n", numUsers, MAXUSERS);
		}
		else{
			bzero(&echoClntAddr, sizeof(struct sockaddr_in));
			bzero(requestMsgBuf, MSGSIZE);
			cliAddrLen = sizeof(echoClntAddr);
			printf("made it here");
			if((recvMsgSize = recvfrom(sock, requestMsgBuf, MSGSIZE, 0, (struct sockaddr *) &echoClntAddr, cliAddrLen)) < 0){
				DieWithError("recvfrom() failed");
			}
			requestMsgBuf[recvMsgSize] = '\0';
			printf("msg %s\n",requestMsgBuf);
			/* check to see what the client wants to do */
			if(strncmp("broadcast", requestMsgBuf, 9) == 0){
				strcat(sendMsgBuf, "type your message (max 1024 characters)");
				if((sendto(sock, sendMsgBuf, MSGSIZE, 0, (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr))) < 0){
					DieWithError("sendto() failed");
				}

			}
			else if(strncmp("private", requestMsgBuf, 7) == 0){
				
			}
		}
	}
/* NOT REACHED */
}
