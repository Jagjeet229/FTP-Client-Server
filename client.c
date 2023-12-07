#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <stdio.h>


  
int connectClient (char *svName,int *clientSocketNo) //This function takes 2 arguments, 1st is the ip address notation of sv and 2nd being the control connection socket number.
{
	int tempSock; //temp variable in order to store the socket number and send it at the end to main function via referencing.
	struct sockaddr_in clientAdd;  	// variable to store client ip address
	struct sockaddr_in svAdd;	// variable to store sv ip address
	struct hostent *svIP;	//this variable stores the server ip structure

	if((svIP = gethostbyname(svName)) == NULL) // here we get the sv ip structure from the sv name using gethosbyname function.
	// the gethostbyname method obtains host data associated with a host name.
	{
		//it enters here and print unknown host error if we are not able to retreive the data corresponding to svName.
		printf("Unknown host error");
		return (-1);  
	}
	// It is to request a reliable connection stream over the Internet.
	if((tempSock = socket(AF_INET, SOCK_STREAM, 0)) < 0) //socket() creates an endpoint for communication and returns a file descriptor referencing the socket.
		//it is used to create a control connection socket.
	{	
		//the code enters here incase there is error in creating the socket and prints the following error.
		printf("Socket not created error ");
		return (-1);	
	}
	
	memset((char *) &clientAdd, 0, sizeof(clientAdd)); //it initializes the client address structure to 0.
	clientAdd.sin_family = AF_INET;	//This sets the sin family to Internet protocol family
	clientAdd.sin_addr.s_addr = htonl(INADDR_ANY); //this will allow the system to fill the ip because INADDR_ANY sets it to any(0). 
	clientAdd.sin_port = 0; //with setting it to 0, system will be able to allocate any free port.

	if(bind(tempSock,(struct sockaddr *)&clientAdd,sizeof(clientAdd))<0) //The bind() function binds a unique local name to the socket with descriptor socket
	{
		//code enters here if the bind function returns -1 which means it was not able to bind and then we print the following in case of error.
		printf("Not able to bind - error");
		close(tempSock);
		return(-1);	
	}
	memset((char *) &svAdd, 0, sizeof(svAdd)); //it initializes the server address structure to 0.
	// Set ftp server ftp address in serverAddress 
	svAdd.sin_family = AF_INET;
	memcpy((char *) &svAdd.sin_addr, svIP->h_addr, 
			svIP->h_length); // copying svIP->h_addr into svAdd.sin_addr
	svAdd.sin_port = htons(1231); //htons converts host byte order into network byte order 

	if (connect(tempSock, (struct sockaddr *) &svAdd, sizeof(svAdd)) < 0)//The connect function establishes a connection to a specified socket
	{	//if code enters here it means that we were not able to establish the connection and then we print the following.
		printf("Cannot connect to server ");
		close (tempSock); 	
		return(-1);  	
	}
	*clientSocketNo=tempSock;
	return(0); 
} 

int sendMsg(int clientSocketNo,char *inputCmd,int msgLen) //this function is used to send the message to the server where is connected to socket number passed as 1st argument.
//In this function, 1st argument is the socket number where the message has to be sent, 2nd argument is the message and 3rd is the length of the message.
{
	if((send(clientSocketNo,inputCmd,msgLen,0)) < 0) //The send() method will start sending a message to its peer from the provided socket.
	{	//if code enters here it means we were not able to send the message.
		printf("Error not able to send message");
		return(-1);
	}
	return(0);
}
int receiveMsg (int clientSocketNo, char *buffer,int bufferSize,int *msgLen) //this function will receive the reply message from the server to the client.
{ //This function has 4 arguments, 1st being the socket number from which message is received, 2nd being the buffer storing the reply message, 3rd being the buffer size and 4th is the length of message received.
	int i;
	*msgLen=recv(clientSocketNo,buffer,bufferSize,0); //Reading receiving data from connection-oriented or connectionless sockets is done using the recv function.
	
	if(*msgLen<0)
	{	//code enters this incase we are not able to receive the message or we encountered an error while receiving the message.
		printf("Error not able to receive message");
		return(-1);
	}
	
	printf("%s \n",buffer); //printing the reply received from the server
	return (0);
}
  

int getClientDatasocket (int *clientDatasocket) //this function is used to create a connection and send/receive datasocket when STOR/RETR command is used.
{
  int tempSock; //temp variable in order to store the socket number and send it at the end to main function via referencing.
	struct sockaddr_in svAdd; // variable to store sv ip address
	// It is to request a reliable connection stream over the Internet.
	if( (tempSock=socket(AF_INET, SOCK_STREAM,0)) <0) //socket() creates an endpoint for communication and returns a file descriptor referencing the socket.
	//it is used to create a control connection socket.
	{	//the code enters here incase there is an error while creating a socket and the error is printed as follows.
		printf("Socket not created error ");
		return(-1);
	}

	memset((char *)&svAdd,0, sizeof(svAdd));
	svAdd.sin_family = AF_INET;
	svAdd.sin_addr.s_addr=htonl(INADDR_ANY);
	svAdd.sin_port=htons(1232);
	if(bind(tempSock,(struct sockaddr *)&svAdd,sizeof(svAdd))<0) //The bind() function binds a unique local name to the socket with descriptor socket
	{
		//code enters here incase there is an error while binding and then the error is printed as follows.
		printf("Not able to bind error");
		close(tempSock);
		return(-1);	
	}
	
	listen(tempSock,1);  //It builds a connection request queue of length backlog to queue incoming connection requests and signals ready to accept client connection requests.
	*clientDatasocket=tempSock;//putting tempSock into clientDatasocket passed as reference.
	return(0); 
}
int main(int argc,char *argv[])
{	char inputCmd[2048];	// It will store the command entered by user
	char cmdExt[2048];		//It is used to extract the FTP command from user input
	char param[2048];	// It stores the parameter extracted from user input
	char receivedMsg[4096]; // It will store the message received from server as a reply
	int clientSocket;	//It stores the socket used for client communication
	int msgLen;	// It stores the length of received message as a reply
	int flag = 0; //flag variable to check if connection is made or error is returned
	printf("***********FTP CLIENT STARTED***********\n");
	printf("****************************************\n");	 
	printf("Connecting to server\n");	
	flag=connectClient("127.0.0.1", &clientSocket);  //calling the connecClient function and the local ip address to form a connection to the server.
	if(flag != 0)
	{	//if flag is not 0 that means there was an error while connecting to the server and hence the following error is printed as follows.
		printf("Not able to connect to server \n");
		return (flag);
	}
	do
	{ //infinite loop to keep taking input from client and printing the the output received from server on the screen
		printf("FTP(Enter command)> ");
		gets(inputCmd); //take input from client screen
		flag = sendMsg(clientSocket, inputCmd, strlen(inputCmd)+1); //send the message taken as input to the server
		if(flag != 0)
		{
		    break;
		}
    if(strstr(inputCmd, " ")!=NULL) { //this checks if there are spaces in the command entered by user .
      strcpy(cmdExt, strtok(inputCmd, " ")); //extracting the 1st part of command
      strcpy(param, strtok(NULL, " ")); //extracting 2nd part of command that is the parameter
      if(strcmp("STOR", cmdExt)==0 || strcmp("stor",cmdExt)==0) //Checking if the command entered is STOR command by the user
	  {
        FILE *f1; //var to store file pointer
        char buff[1024]; //buffer to store data
        int totalBytes=0; //no of bytes read
        int clientDatasocket; //variable to store socket number when connection is being established
        flag = getClientDatasocket(&clientDatasocket); //calling getclientdatasocket function to create a connection to send/receive file data
        if(flag!=0){
			//incase there is an error occurred while connecting the following error is printed
			printf("Error occured while receiving client data socket");
		} 
        clientDatasocket=accept(clientDatasocket, NULL, NULL); //The accept() function will extract the first connection on the queue of pending connections
       
        f1=fopen(param, "r"); //open the file with filename "param" in read mode.
        if(f1!=NULL) { //check if we are able to open the file or not
          while(!feof(f1)) {
            totalBytes=fread(buff, sizeof(char), 1023, f1); //read the bytes from file
            flag = sendMsg(clientDatasocket, buff, strlen(buff)+1); //calling the send msg function
            if(flag!=0) break; //checking if sending mssg is successfull or not
          }
          close(clientDatasocket); //closing the socket
          fclose(f1); //closing the file
        } else { //if the filename entered is not there, print the following error
          sprintf(receivedMsg, "Error file doesnt exist");
          close(clientDatasocket); //close the socket
        }
      }
      else if(strcmp("RETR", cmdExt)==0 || strcmp("retr", cmdExt)==0) //checking if the command entered is RETR command by the user
	  {
        char buff[1024]; //variable to store content
        int clientDatasocket; //variable to store socket number
        flag = getClientDatasocket(&clientDatasocket); //calling getclientdatasocket function to create a connection to send/receive file data
        if(flag!=0){// incase there is an error while connecting, the following error is printed
			printf("Error occured while receiving client data socket");
		} 
        clientDatasocket=accept(clientDatasocket, NULL, NULL); //The accept() function will extract the first connection on the queue of pending connections
        FILE *f2=fopen(param, "w"); //opening file in write mode
        if(f2==NULL) { //incase we are not able to open the file
          while(1) {
            flag=receiveMsg(clientDatasocket,buff,sizeof(buff), &msgLen); //receive the reply from server
            if(flag!=0 || msgLen ==0) break;
            fwrite(buff, sizeof(char), msgLen, f2); //writing the reply received to the file
            fflush(f2);
            memset(buff,'\0',sizeof(buff)); //initializing buff back to \0
          }
        } else {
          sprintf(receivedMsg, "Error not able to receive");
        }
        close(clientDatasocket); //closing socket
        fclose(f2); //closing the file
      }
    }	
		flag = receiveMsg(clientSocket, receivedMsg, sizeof(receivedMsg), &msgLen);	 //receive message is called to receive the reply from the server and print it
		if(flag != 0)
		{	//incase there is an error receiving the reply, we break out of the infinite loop
		    break;
		}
    } while(strcasecmp(inputCmd,"quit")!=0); //break out of the infinite loop incase quit command is entered by the user
	printf("Control connection between server client is closed \n");
	close(clientSocket); //close the socket
	return (flag); 
}

