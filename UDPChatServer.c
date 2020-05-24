/* This file created by Mark Battle is the server side of the UDP Chat program. This side is run
 * by calling make from the makefile and then running ./UDPChatServer with a port number as an
 * argument so it would be like "./UDPChatServer 1234" in the commandline. 
 * The server then continuously receives and sends messages to and from the client.
 * The server checks to see if the client already exists. If they do not, they add them
 * to the list of clients.
 * Depending on what the message is, the server runs a different set of instructions:
 * If the client sends the message broadcast, it will send out the next typed message to
 * all of the clients currently in the chatroom.
 */
#include <stdio.h> /*for printf() and fprintf()*/
#include <sys/socket.h> /*for socket(), connect(), send(), and recv()*/
#include <arpa/inet.h> /*for sockaddr_in and inet_addr()*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NAMEMAX 32   /* Longest string for username */
#define MAXUSERS 3   /* the number of users allowed to enter the chatroom */
#define MSGSIZE 1024 /* size of message that can be sent */
#define TRUE 1	 
#define FALSE 0

void DieWithError(char *errorMessage); /* Error handling function */

/* struct to hold the clients' information on the server */
typedef struct client{
	struct sockaddr_in clntAddr; /*client address*/
	char username[NAMEMAX]; /* hold clients username initialized with first connection */
} client_t;

/* Compares 2 addresses to determine if they are the same. returns true if they are
 * and returns false if they are not
 */
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

/* Main program ran. Runs as described in the first few lines of this file */
int main(int argc, char *argv[])
{

	int sock; /* Socket */
	struct sockaddr_in chatServAddr; /* Local address */
	struct sockaddr_in chatClntAddr;
	struct sockaddr_in sendToAddr;
	unsigned int cliAddrLen; /* Length of incoming message */
	char sendToUser[NAMEMAX]; 
	char requestMsgBuf[MSGSIZE]; /* buffer that is being received from the client by the server */
	char sendMsgBuf[MSGSIZE+NAMEMAX+11]; /* buffer that is being sent from the server to a client: have to have enough space for the message size, the username, and the string "[private]:" */
	client_t clntList[MAXUSERS]; /* list of clients connected to the server in the chatroom */
	int clientExists; /* used to determine if the client already exists in the chatroom */
	int numUsers; /* keep track of the total number of users in the chatroom */
	int iter; /* used to iterate through the list of clients */
	unsigned short chatServPort; /* Server port */
	int recvMsgSize; /* Size of received message */

	if (argc != 2) /* Test for correct number of arguments */
	{
		fprintf(stderr, "Usage: %s <Server Port>\n", argv[0]) ;
		exit(1);
	}

	chatServPort = atoi(argv[1]); /* First arg: local port */

	/* Create socket for incoming connections */
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		DieWithError( "socket () failed") ;

	/* Construct local address structure */
	memset(&chatServAddr, 0, sizeof(chatServAddr)); /* Zero out structure */
	chatServAddr.sin_family = AF_INET; /* Internet address family */
	chatServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
	chatServAddr.sin_port = htons(chatServPort); /* Local port */

	/* Bind to the local address */
	if (bind(sock, (struct sockaddr *)&chatServAddr, sizeof(chatServAddr)) < 0)
		DieWithError ( "bind () failed");


	numUsers = 0;
	for (;;) /* Run forever */
	{
		bzero(sendMsgBuf, MSGSIZE);
		cliAddrLen = sizeof(chatClntAddr);
		/*receive the names of the users and store their information
		 *  in a buffer to be used for later
		 */
		if((recvMsgSize = recvfrom(sock, requestMsgBuf, MSGSIZE, 0, (struct sockaddr *) &chatClntAddr, &cliAddrLen)) < 0){
				DieWithError("recvfrom() failed");
			}
		clientExists = FALSE;
		requestMsgBuf[recvMsgSize] = '\0';
		for(iter=0;iter<=numUsers;iter++){
			if(clientCompare(clntList[iter].clntAddr, chatClntAddr) == TRUE){
				clientExists = TRUE;
			}
		}
		if(numUsers == 0){
			bzero(sendMsgBuf, MSGSIZE+NAMEMAX+11);
			clntList[0].clntAddr = chatClntAddr;
			strcpy(clntList[0].username, requestMsgBuf);
			strcpy(sendMsgBuf, "Username");
			
			if(sendto(sock, sendMsgBuf, MSGSIZE+NAMEMAX+11, 0, &clntList[0].clntAddr, sizeof(clntList[0].clntAddr)) < 0){
				DieWithError("sendto() failed");
			}


			if(sendto(sock, clntList[0].username, NAMEMAX, 0, &clntList[0].clntAddr, sizeof(clntList[0].clntAddr)) < 0){
				DieWithError("sendto() failed");
			}
			printf("new user added: %s\n", clntList[0].username);
			numUsers++;
		}
		/* if client doesn't exist, add them to the list of clients connected to the server */
		else if(!clientExists){
			bzero(sendMsgBuf, MSGSIZE+NAMEMAX+11);
			clntList[numUsers].clntAddr = chatClntAddr;
			strcpy(sendMsgBuf, "Username");
			strcpy(clntList[numUsers].username, requestMsgBuf);

			printf("new user added: %s\n", clntList[numUsers].username);
			for(int j = 0;j<=numUsers;j++){
				for(int k = 0; k<=numUsers; k++){
					if(sendto(sock, sendMsgBuf, MSGSIZE+NAMEMAX+11, 0, &clntList[j].clntAddr, sizeof(clntList[j].clntAddr)) < 0){
						DieWithError("sendto() failed");
					}

					if(sendto(sock, clntList[k].username, MSGSIZE, 0, &clntList[j].clntAddr, sizeof(clntList[j].clntAddr)) < 0){
						DieWithError("sendto() failed");
					}
				}
			}
			numUsers++;		
		}
		/* go through the broadcast protocol. ask the user to enter their message, receive the message
		 * then send it out to all the other users in the chat, skipping over the user that sent the
		 * the message.
		 */
		if(strcmp("broadcast\n", requestMsgBuf) == 0){
			bzero(sendMsgBuf, MSGSIZE+NAMEMAX+11);
			strcpy(sendMsgBuf,"enter your message");
			if(sendto(sock, sendMsgBuf, MSGSIZE+NAMEMAX+11, 0, &chatClntAddr, sizeof(chatClntAddr)) < 0){
				DieWithError("sendto() failed");
			}
			if((recvMsgSize = recvfrom(sock, requestMsgBuf, MSGSIZE, 0, (struct sockaddr *) &chatClntAddr, &cliAddrLen)) < 0){
				DieWithError("recvfrom() failed");
			}
			requestMsgBuf[recvMsgSize] = '\0';
			bzero(sendMsgBuf, MSGSIZE+NAMEMAX+11);
			for(iter = 0;iter<numUsers;iter++){
				if(clientCompare(clntList[iter].clntAddr, chatClntAddr) == TRUE){
					strcpy(sendMsgBuf, clntList[iter].username);
					strcat(sendMsgBuf, ": ");
					strcat(sendMsgBuf, requestMsgBuf);
				}
			}
			for(iter = 0;iter<numUsers;iter++){
				if(clientCompare(clntList[iter].clntAddr, chatClntAddr) == FALSE){
					if(sendto(sock, sendMsgBuf, MSGSIZE+NAMEMAX+11, 0, &clntList[iter].clntAddr, sizeof(clntList[iter].clntAddr)) < 0){
						DieWithError("sendto() failed");
					}
				}
			}


		}
		/* Goes through the private message protocol
		 * first asks for the username of the recipient of the message
		 * then asks for the message
		 * then sends the message beginning with the name of the user
		 * plus saying that it is a private message
		 */
		else if(strcmp("private\n", requestMsgBuf) == 0){
			bzero(sendMsgBuf, MSGSIZE+NAMEMAX+11);
			strcpy(sendMsgBuf, "enter the name of the user you would like to send the message to:");
			if(sendto(sock, sendMsgBuf, MSGSIZE+NAMEMAX+11, 0, &chatClntAddr, sizeof(chatClntAddr)) < 0){
				DieWithError("sendto() failed");
			}
			if((recvMsgSize = recvfrom(sock, requestMsgBuf, MSGSIZE, 0, (struct sockaddr *) &chatClntAddr, &cliAddrLen)) < 0){
				DieWithError("recvfrom() failed");
			}
			requestMsgBuf[recvMsgSize] = '\0';
			bzero(sendToUser, NAMEMAX);
			strcpy(sendToUser, requestMsgBuf);
			sendToUser[strlen(sendToUser)-1] = '\0'; //gets rid of the newline char at the end so the strcmp later works with clntList[iter].username
			bzero(sendMsgBuf, MSGSIZE+NAMEMAX+11);
			strcpy(sendMsgBuf, "Enter the message: ");
			if(sendto(sock, sendMsgBuf, MSGSIZE+NAMEMAX+11, 0, &chatClntAddr, sizeof(chatClntAddr))<0){
				DieWithError("recvfrom() failed");
			}
			bzero(sendMsgBuf, MSGSIZE+NAMEMAX+11);
			for(iter = 0; iter<numUsers; iter++){
				if(clientCompare(clntList[iter].clntAddr, chatClntAddr) == TRUE){
					strcpy(sendMsgBuf, clntList[iter].username);
					strcat(sendMsgBuf, "[private]: ");
				}
			}
			bzero(requestMsgBuf, MSGSIZE);
			if((recvMsgSize = recvfrom(sock, requestMsgBuf, MSGSIZE, 0, (struct sockaddr *) &chatClntAddr, &cliAddrLen))<0){
				DieWithError("recvfrom() failed");
			}
			requestMsgBuf[recvMsgSize] = '\0';
			strcat(sendMsgBuf, requestMsgBuf);
			for(iter=0; iter<numUsers; iter++){
				if(strcmp(sendToUser, clntList[iter].username) == 0){
					if(sendto(sock, sendMsgBuf, MSGSIZE+NAMEMAX+11, 0, &clntList[iter].clntAddr, sizeof(clntList[iter].clntAddr)) < 0){
						DieWithError("sendto() failed");
					}
				}
			}
		}			
	}
/* NOT REACHED */
}
