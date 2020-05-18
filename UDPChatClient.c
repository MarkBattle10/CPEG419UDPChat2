#include <stdio.h> /*for printf() and fprintf()*/
#include <sys/socket.h> /*for socket(), connect(), send(), and recv()*/
#include <arpa/inet.h> /*for sockaddr_in and inet_addr()*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//#include <pthread.h>
#include <sys/time.h>
#include <sys/select.h> 
#include <sys/types.h> /*for FD_SET*/

#define NAMEMAX 255 /*Longest string to echo*/
#define MAXUSERS 3
#define MAXMSG 1024
#define TRUE 1
#define FALSE 0

void DieWithError(char *errorMessage);
/*
int end = FALSE;

int sock;
struct sockaddr_in echoServAddr;
struct sockaddr_in fromAddr;
unsigned short echoServPort;

char usersBuffer[MAXUSERS][NAMEMAX];

pthread_mutex_t mutexsum = PTHREAD_MUTEX_INITIALIZER;
*/
//void *sender(); /* thread function that will take user input and send out messages to server */

//void *receiver(); /* thread function that will listen for received messages coming from the server */

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
	fd_set rfds;
	struct timeval tv;
	int retval;

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

	
	/* Watch stdin (fd 0) to see when it has input */
	FD_ZERO(&rfds);
	
	/* Continuously switch: no wait time */
	tv.tv_sec = 0; //seconds
	tv.tv_usec = 0; //microseconds

	/*send the string to the server*/
	if (sendto(sock, userName, userNameLen, 0, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr))!=userNameLen)
		DieWithError("send() sent a different number of bytes than expected");
	fromSize = sizeof(fromAddr);
	numUsers = 0;
	for(int j = 0;j<MAXUSERS;j++){
		usersBuffer[j][0] = 0;
	}

	for(;;){
		FD_SET(0,&rfds); //watching stdin
		FD_SET(sock, &rfds); //watching the socket
		retval = select(sock+1, &rfds, NULL, NULL, &tv);

		if(retval == -1){
			DieWithError("select() failed");
		}
		else if(retval){
			if(FD_ISSET(0, &rfds)){
				fgets(sendMsgBuf, MAXMSG, stdin);
				//printf("%s\n",sendMsgBuf);
		
				if (sendto(sock, sendMsgBuf, MAXMSG, 0, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr))<0)
					DieWithError("send() sent a different number of bytes than expected");

			}
			else if(FD_ISSET(sock, &rfds)){
				cliExists=FALSE;
				if((respStringLen = recvfrom(sock, recvMsgBuf, MAXMSG, 0, (struct sockaddr *) &fromAddr, &fromSize)) < 0){
					DieWithError("recv() failed or connection closed prematurely");
				}
				recvMsgBuf[respStringLen] = '\0';
				//printf("strcmp: %d\n", strcmp("Username",recvMsgBuf));
				if(strcmp("Username",recvMsgBuf) == 0){	
					if((respStringLen = recvfrom(sock, recvMsgBuf, MAXMSG, 0, (struct sockaddr *) &fromAddr, &fromSize)) < 0){
						DieWithError("recv() failed or connection closed prematurely");
					}

					for(int j=0;j<=numUsers;j++){
						if(strcmp(usersBuffer[j],recvMsgBuf) == 0){
							cliExists = TRUE;
						}
					}
			
					if(!cliExists){
						printf("New User Added: ");
						strcpy(usersBuffer[numUsers], recvMsgBuf);
						printf("%s\n", usersBuffer[numUsers]);
						numUsers++;
						//printf("numUsers: %d\n",numUsers);
					}
				}
				else{
					printf("%s\n",recvMsgBuf); 
				}

			}
		}

	}
	





	/* create two threads
	 * Thread 1: takes in user input and sends out messages
	 * Thread 2: receives messages from the server and prints them to screen
	 */

	/*
	pthread_t threads[2];
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	pthread_create(&threads[0], &attr, sender(), NULL);
	pthread_create(&threads[1], &attr, receiver(), NULL);

	while(!end);
	*/
	close(sock);
	exit(0);

}	
	
/*
void *sender(){
	char sendMsgBuf[MAXMSG];
	//}
*/	/*Send messages:
	 * type "broadcast" followed by your message to send a message
	 * to all other users
	 * type "private" followed by the user you would like to
	 * privately chat
	 */
/*	for(;;){
		pthread_mutex_unlock(&mutexsum);
		printf("made it here 3");
		fgets(sendMsgBuf, MAXMSG, stdin);
		printf("%s\n",sendMsgBuf);
		
		if (sendto(sock, sendMsgBuf, MAXMSG, 0, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr))<0)
			DieWithError("send() sent a different number of bytes than expected");
		
		printf("made it here");

		pthread_mutex_unlock(&mutexsum);
	}
*/	/*	
		int charIter = 0;
		while((sendMsgBuf[charIter++] = getchar())!= '\n');
		printf("%s\n", sendMsgBuf);
		if (sendto(sock, sendMsgBuf, strlen(sendMsgBuf), 0, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr))<0)
			DieWithError("send() sent a different number of bytes than expected");
	//if((respStringLen = recvfrom(sock, recvMsgBuf, MAXMSG, 0, (struct sockaddr *) &fromAddr, &fromSize)) < 0){
		//	DieWithError("recv() failed or connection closed prematurely");
		//}

	*/



	/*Receive a response*/
	/*
	fromSize = sizeof(fromAddr);
	if ((respStringLen = recvfrom(sock, echoBuffer, NAMEMAX, 0, (struct sockaddr *) &fromAddr, &fromSize)) != userNameLen)
		DieWithError("recvfrom() failed") ;
	*/
/*}

void *receiver(int sock, ){
	char recvMsgBuf[MAXMSG];
	int numUsers;
	unsigned int fromSize;
	int cliExists;
	int respStringLen;


	fromSize = sizeof(fromAddr);
	numUsers = 0;
	for(int j = 0;j<MAXUSERS;j++){
		usersBuffer[j][0] = 0;
	}
	//printf("Current Users: \n");
	for(;;){
		pthread_mutex_unlock(&mutexsum);
*/		/*receive the usernames of the users in the chat*/
		//if(numUsers<MAXUSERS){
/*		cliExists=FALSE;
		if((respStringLen = recvfrom(sock, recvMsgBuf, MAXMSG, 0, (struct sockaddr *) &fromAddr, &fromSize)) < 0){
			DieWithError("recv() failed or connection closed prematurely");
		}
		recvMsgBuf[respStringLen] = '\0';
			
		for(int j=0;j<=numUsers;j++){
			if(strcmp(usersBuffer[j],recvMsgBuf) == 0){
				cliExists = TRUE;
			}
		}
			
		if(!cliExists){
			printf("New User Added: ");
			strcpy(usersBuffer[numUsers], recvMsgBuf);
			printf("%s\n", usersBuffer[numUsers]);
			numUsers++;
			//printf("numUsers: %d\n",numUsers);
		}
		printf("made it here 2");
		//pthread_mutex_unlock(&mutexsum);
	}


}
*/
/*
	if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
	{
		fprintf(stderr,"Error: received a packet from unknown source.\n");
		exit(1);
	}
*/	

