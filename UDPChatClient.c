/* This file created by Mark Battle is the client side of the UDP Chat program. This side is run
 * by calling make from the makefile and then running ./UDPChatClient with an ip address, username,
 * and optional port number as arguments so it would look like "./UDPChatClient 127.0.0.1 FirstUser 1234"
 * in the commandline
 * The client continuously sends and receives messages such as when a new user enters the chatroom.
 * The client has to specify whether or not it is broadcasting its message or sending a private message
 * to another user in the chatroom
 */

#include <stdio.h> /*for printf() and fprintf()*/
#include <sys/socket.h> /*for socket(), connect(), send(), and recv()*/
#include <arpa/inet.h> /*for sockaddr_in and inet_addr()*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//#include <pthread.h>
#include <sys/time.h> /* struct timeval tv */
#include <sys/select.h> /* for select() */
#include <sys/types.h> /*for FD_SET*/

#define NAMEMAX 255 /*Longest string to echo*/
#define MAXUSERS 3
#define MAXMSG 1024
#define TRUE 1
#define FALSE 0

void DieWithError(char *errorMessage);

/* attempted to have the client receive and send messages at the same time using mutual exclusion switching */
/*
pthread_mutex_t mutexsum = PTHREAD_MUTEX_INITIALIZER;
*/
//void *sender(); /* thread function that will take user input and send out messages to server */

//void *receiver(); /* thread function that will listen for received messages coming from the server */

int main(int argc, char *argv[])
{
	int sock; /*Socket descriptor*/
	struct sockaddr_in chatServAddr; /* Chat server address */
	struct sockaddr_in fromAddr; /* Source address of server */
	unsigned short chatServPort; 
	unsigned int fromSize;
	char *servIP; /* to get the server IP address from the commandline argument argv[1] */
	char *userName; /* to get the username from the commandline argument argv[2] */
	char usersBuffer[MAXUSERS][NAMEMAX]; /* to store the usernames of all the users in the chatroom */
	int numUsers; /* to keep track of the number of users in the chatroom */
	int cliExists; /* a variable to check if the client exists or not when a new user is added */
	unsigned int userNameLen; /* to get the length of the username */
	int respStringLen; /* length of the string received in the buffer from the server */
	char sendMsgBuf[MAXMSG]; /* buffer to send to the server obtained from fgets stdin */
	char recvMsgBuf[MAXMSG]; /* message received from the server */
	fd_set rfds; /* read the fd set to see if characters become available for reading (in our case check if a user is typing in stdin) */
	struct timeval tv; /* the timeout argument for select() is a struct that has time in seconds and microseconds */
	int retval; /* the output of the select() function is an integer that will be stored in retval */

	if ((argc<3)||(argc>4))
	{
			fprintf(stderr, "Usage: %s <Server IP> <Username> [<Chat Port>]\n"), argv[0];
			exit(1);
	}

	servIP = argv[1];
	userName = argv[2];
	
	if ((userNameLen = strlen(userName)) > NAMEMAX) /* Check input length */
		DieWithError("Username too long");

	if(argc == 4)
		chatServPort = atoi(argv[3]);
	else
		chatServPort = 7;

	/* Create a datagram/UDP socket */
	if((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP))<0)
		DieWithError("socket() failed");

	/*Construct the server address structure*/
	memset(&chatServAddr, 0, sizeof(chatServAddr)); /* Zero out structure */
	chatServAddr.sin_family = AF_INET;   /* Internet addr family */
	chatServAddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
	chatServAddr.sin_port = htons(chatServPort);  /* Server port */

	
	/* Watch stdin (fd 0) to see when it has input */
	FD_ZERO(&rfds);
	
	/* Continuously switch: no wait time */
	tv.tv_sec = 0; //seconds
	tv.tv_usec = 0; //microseconds

	/*send the string to the server*/
	if (sendto(sock, userName, userNameLen, 0, (struct sockaddr *)&chatServAddr, sizeof(chatServAddr))!=userNameLen)
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
			/* send messages to the server by reading in the stdin */
			if(FD_ISSET(0, &rfds)){
				fgets(sendMsgBuf, MAXMSG, stdin);
				if (sendto(sock, sendMsgBuf, MAXMSG, 0, (struct sockaddr *) &chatServAddr, sizeof(chatServAddr))<0)
					DieWithError("send() sent a different number of bytes than expected");

			}
			/* receive messages from the server */
			else if(FD_ISSET(sock, &rfds)){
				cliExists=FALSE;
				if((respStringLen = recvfrom(sock, recvMsgBuf, MAXMSG, 0, (struct sockaddr *) &fromAddr, &fromSize)) < 0){
					DieWithError("recv() failed or connection closed prematurely");
				}
				recvMsgBuf[respStringLen] = '\0';
				/* if the received buffer is "Username" then inform the user that recieves
			 	 * this message that a new user is being added to the chatroom and
				 * then receive the name of the new user
				 */
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
						printf("Type \"broadcast\" and hit enter to send a message to everyone in the chatroom\n Type \"private\" to send a message to a single user in the chatroom\n");
					}
				}
				/* if the message isn't "Username", just print out the received
				 * message from the server (usually instructions on what to type next)
				 */
				else{
					printf("%s\n",recvMsgBuf); 
				}

			}
		}

	}
	close(sock);
	exit(0);	
}
	
/* The continuation of my attempt at using mutual exclusion to switch between reading stdin
	 * and receiving messages from the server
	 */
	
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
	
}	
*/	
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

