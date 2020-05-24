Protocol design: 
  There are three protocols for this chat program which are broadcast message, private message, and add user (I could not get the file transfer to work so I omitted it)
  1. When the user wants to broadcast a message, they must type the word broadcast in and press enter. The server will then reply prompting
  them to enter their message. The server then receives the message and sends it out to everyone except the person who sent it.
  2. When the user wants to send a private message, they must type the word private in and press enter. The server will then reply promping
  the user to enter the name of the user they would like to send the private message to. The client replies with the name and the server then
  asks for the message. The server puts the name of the sender, the fact that its a private message, and the message together in a buffer and
  sends it to the intended user.
  3. When a new user is added. The server sends the message "username" to all the users which tells them they are going to be getting a message
  about a new user. The clients then wait for the message and check if it is a new user and if it is print out that a new user has been added.
  
List of Relevant Files:
  The relevant files include UDPChatClient.c, UDPChatServer.c, DieWithError.c, and the makefile
  
Compilation Instructions:
  To compile this code, simply type in the command line "make" and the code should be compiled ready to be run using the executables.
  The executables generated will be UDPChatClient and UDPChatServer. You must run UDPChatServer first.
  1. to run UDPChatServer, type "./UDPChatServer" followed by a port number as an argument so it would look like this(cut paste the next quoted section):
      "./UDPChatServer 1234"
  2. to run UDPChatClient, type "./UDPChatClient" followed by an local IP address, your chosen username, and a port number, so it would look
     like this(cut paste the next quoted section):
     "./UDPChatClient 127.0.0.1 FirstUser 1234" 
     where the port number matches the server's port number.

Configuration File:
  The only configuration file I have is a makefile. The makefile simply compiles all of my source files using gcc.
  
Running Instructions:
  As described in the compilation instructions:
    The executables generated will be UDPChatClient and UDPChatServer. You must run UDPChatServer first.
    1. to run UDPChatServer, type "./UDPChatServer" followed by a port number as an argument so it would look like this(cut paste the next quoted section):
       "./UDPChatServer 1234"
       This is the server for the clients to connect with
    2. to run UDPChatClient, type "./UDPChatClient" followed by an local IP address, your chosen username, and a port number, so it would look
       like this(cut paste the next quoted section):
       "./UDPChatClient 127.0.0.1 FirstUser 1234" 
       where the port number matches the server's port number.
       This is the clients that connect and send/receive messages with the server.
     
