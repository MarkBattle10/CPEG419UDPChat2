CFLAGS=-Wall

all: UDPChatClient UDPChatServer

UDPChatClient :	UDPChatClient.o	DieWithError.o  

UDPChatServer :	UDPChatServer.o	DieWithError.o 

DieWithError.o : DieWithError.c
				gcc -c DieWithError.c 

UDPChatClient.o: UDPChatClient.c
				gcc -c UDPChatClient.c

UDPChatServer.o: UDPChatServer.c 
				gcc -c UDPChatServer.c 

clean:
		rm -f	UDPChatClient.o	DieWithError.o UDPChatServer.o UDPChatClient UDPChatServer
				
