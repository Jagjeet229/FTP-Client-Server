#include <stdio.h>
#include <sys/socket.h>
#include<dirent.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include<time.h>
#include <stdlib.h>
#include <netdb.h>

int dataConnection (char *svIP,int *svSocketNo) //this function is used to create a connection and send/receive datasocket when STOR/RETR command is used.
{
	int tempSock;	//temp variable to store socket no
	struct sockaddr_in clientAddress;  	//variable to store client ip address
	struct sockaddr_in serverAddress;	//variable to store server ip address
	struct hostent	*serverIPstructure;	//this variable stores the server ip structure
	if((serverIPstructure = gethostbyname(svIP)) == NULL) // here we get the sv ip structure from the sv name using gethosbyname function.
	// the gethostbyname method obtains host data associated with a host name.
	{	
		//it enters here and print unknown host error if we are not able to retreive the data corresponding to svName.
		printf("Unknown host error \n");
		return (-1);  
	}
	// It is to request a reliable connection stream over the Internet.
	if((tempSock = socket(AF_INET, SOCK_STREAM, 0)) < 0) //socket() creates an endpoint for communication and returns a file descriptor referencing the socket.
	//it is used to create a control connection socket.
	{ //the code enters here incase there is error in creating the socket and prints the following error.
		printf("Socket not created error");
		return (-1);	
	}

	memset((char *) &clientAddress, 0, sizeof(clientAddress)); //it initializes the client address structure to 0.
	clientAddress.sin_family = AF_INET;		//This sets the sin family to Internet protocol family
	clientAddress.sin_addr.s_addr = htonl(INADDR_ANY);//this will allow the system to fill the ip because INADDR_ANY sets it to any(0).   	
	clientAddress.sin_port = 0; //with setting it to 0, system will be able to allocate any free port.
 
	if(bind(tempSock,(struct sockaddr *)&clientAddress,sizeof(clientAddress))<0) //The bind() function binds a unique local name to the socket with descriptor socket
	{
		//code enters here if the bind function returns -1 which means it was not able to bind and then we print the following in case of error.
		printf("Not able to bind error");
		close(tempSock);
		return(-1);	
	}

	memset((char *) &serverAddress, 0, sizeof(serverAddress)); //it initializes the server address structure to 0.
	// Set ftp server ftp address in serverAddress 
	serverAddress.sin_family = AF_INET;
	memcpy((char *) &serverAddress.sin_addr, serverIPstructure->h_addr, 
			serverIPstructure->h_length); // copying svIP->h_addr into svAdd.sin_addr
	serverAddress.sin_port = htons(1232); //htons converts host byte order into network byte order 
	if (connect(tempSock, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) //The connect function establishes a connection to a specified socket
	{	//if code enters here it means that we were not able to establish the connection and then we print the following.
		printf("Error= not able to connect to server ");
		close (tempSock); 
		return(-1);  
	}
	*svSocketNo=tempSock;
	return(0); 
}

int initializeSv (int *svSocketNo) //this function initializes the server. It is used to create a socket and then listen using the socket created for any requets from client
{
	int tempSock; //variable to store socket number required for connection establishment
	struct sockaddr_in svAdd; //variable to store server ip address
	
	if( (tempSock=socket(AF_INET, SOCK_STREAM,0)) <0)  //socket() creates an endpoint for communication and returns a file descriptor referencing the socket.
	//it is used to create a control connection socket.
	{	//the code enters here incase there is error in creating the socket and prints the following error.
		printf("Socket not created error");
		return(-1);
	}

	memset((char *)&svAdd,0, sizeof(svAdd)); //it initializes the server address structure to 0.
	svAdd.sin_family = AF_INET; 	//This sets the sin family to Internet protocol family
	svAdd.sin_addr.s_addr=htonl(INADDR_ANY);  //this will allow the system to fill the ip because INADDR_ANY sets it to any(0). 
	svAdd.sin_port=htons(1231);  ; //htons converts host byte order into network byte order 

	if(bind(tempSock,(struct sockaddr *)&svAdd,sizeof(svAdd))<0) //The bind() function binds a unique local name to the socket with descriptor socket
	{	//code enters here if the bind function returns -1 which means it was not able to bind and then we print the following in case of error.
		printf("Not able to bind error");
		close(tempSock);
		return(-1);	
	}
	listen(tempSock,1);  //It builds a connection request queue of length backlog to queue incoming connection requests and signals ready to accept client connection requests.
	
	*svSocketNo=tempSock; //putting tempsock into svSocketNo passed as reference

	return(0); 
}

int sendMsg(int socketNo,char *replyMsg,int msgLen) //this function is used to send the reply to the client where is connected to socket number passed as 1st argument.
{
	if((send(socketNo,replyMsg,msgLen,0)) < 0) //The send() method will start sending a message to its peer from the provided socket.
	{	//if code enters here it means we were not able to send the message.
		printf("Error not able to send message");
		return(-1);
	}
	return(0);
}
int receiveMsg (int socketNo, char *buffer,int bufferSize,int *msgLen) //this function will receive the reply message from the server to the client.
{	//This function has 4 arguments, 1st being the socket number from which message is received, 2nd being the buffer storing the reply message, 3rd being the buffer size and 4th is the length of message received.
	int i;
	*msgLen=recv(socketNo,buffer,bufferSize,0); //Reading receiving data from connection-oriented or connectionless sockets is done using the recv function.
	if(*msgLen<0)
	{	//code enters this incase we are not able to receive the message or we encountered an error while receiving the message.
		printf("Error not able to receive message");
		return(-1);
	}
	
	printf(" Received message from client is: %s",buffer); //printing in the server, the received command from client
	printf("\n");

	return (0);
}

int main(int argc,char *argv[])
{
	char inputCmd[2048]; // It will store the command entered by user
	char cmdExt[2048];		//It is used to extract the FTP command from user input
    char param[2048];	// It stores the parameter extracted from user input
	char receivedMsg[4096];    // It will store the message received from client
	char buffer[4096]; //temp buffer to store data while executing commands in server before sending the reply
	char userAuthList[1024]; // This variable will store the list of users and their passwords
	char user[1024]; //this var will store the username entered by user when input command used is USER
	char pass[1024]; // this var will store the password entered by user when input command is PASS
	char renameFrom[1024]; //variable used to store name of file which has to be renamed when RNFR command is called
	char renameTo[1024]; //variable used to store name to which the file has to be renamed to when RNTO command is called
	int msgLen;        // It stores the length of received message 
	int svSocket;  //It stores the listen socker used by server
	int ctrlSocket;       //It stores the socket used for client communication
	int flag;

	printf("***********FTP Server STARTED***********\n\n");

	flag=initializeSv(&svSocket); //calling the initializeSv function to start the server and listen for any connections
	if(flag != 0)
	{	//Incase there is an error while initializing the following error is printed
		printf("Error while initializing, quitting server\n");
		exit(flag); //exit the function
	}

	printf("Waiting for client to send request for establishment of connection \n");

	ctrlSocket = accept(svSocket, NULL, NULL); //The accept() function will extract the first connection on the queue of pending connections


	if(ctrlSocket < 0)
	{	//Incase there is an error while establishing the connection following error is printed
		printf("Error not able to establish connection, quitting server.\n");
		close(svSocket);  //closing socket
		return (-1); 
	}

	printf("Successfully connected to client. Waiting for FTP commands from client to execute. \n");

	do
	{	//starting infinite loop to receive commands from client and keep replying
 	    flag=receiveMsg(ctrlSocket, inputCmd, sizeof(inputCmd), &msgLen); //receiving message from client
		
	    if(flag < 0)
	    { //Incase there is an error receiving the message, following error is printed
		      printf("Error while receiving the message, quitting .\n");
		      break;
	    }

      	if(strchr(inputCmd,' ')==NULL) strcpy(cmdExt,inputCmd); //it checks if there is a space between received message or not
      	else { //incase there is a space we divide the command into 2 parts and save 1st part in cmdExt and paramter in param
	      		strcpy(cmdExt, strtok(inputCmd, " "));
	      		strcpy(param, strtok(NULL, " "));				
      	   }
     
      	strcpy(userAuthList, "Jagjeet jags1234\n"	//This is the user-pass list created to verify identity when USER/PASS command is used
                     		  "Proudhead proudy\n"
                     		  "Pam december\n"
                     		  "Groot tree\n");

  
	if(strcmp(cmdExt, "pwd")==0 || strcmp(cmdExt, "PWD")==0) { //checking if the command sent by client is PWD
    	memset(buffer,'\0',sizeof(buffer));  //reinitializing buffer back to \0 and this will be done in each of the commands
		getcwd(buffer,sizeof(buffer)); //getting the current working directory of server and storing into buffer
		sprintf(receivedMsg, "Reply by server: 257 %s\n", buffer);	//creating reply string and storing in receivedMsg to send it back to client
	}
  


	else if(strcmp(cmdExt, "LIST")==0 || strcmp(cmdExt, "list")==0) { //checking if the command entered by user is LIST
		memset(receivedMsg,'\0', sizeof(receivedMsg)); //reintializing receivedMsg to \0 so that we can store the reply to send in this variable
		DIR *currDirectory; //variable to store directoryname
		struct dirent *  startPoint; //structure to store foldername 
			char buffer[256]; //variable to store current working directory
			getcwd(buffer, sizeof(buffer)); 
			printf("%s\n", buffer);
			currDirectory=opendir(buffer); //opening the current working directory and storing it in variable directory
			if((currDirectory = opendir(buffer)) == NULL) { //checking if we were able to open the directory or not.
				printf("Error Opening directory");
			}
			strcat(receivedMsg, "Reply by Server : 250 list command executed \n");
			while(( startPoint = readdir(currDirectory)) != NULL) { // loop to check all the files/folders and store their names in receivedMsg
				if((strcmp( startPoint->d_name, ".") != 0) && (strcmp( startPoint->d_name, "..") != 0)) {
					strcat(receivedMsg,  startPoint->d_name);
					strcat(receivedMsg,"\n");
				}
			}
		closedir(currDirectory); //closing the directory
		//reinitializing to \0
		memset(cmdExt,'\0',sizeof(cmdExt));
		memset(param,'\0',sizeof(param));
	}


	else if(strcmp(cmdExt, "MKD")==0 || strcmp(cmdExt, "mkd")==0 ) { //checking if the entered command is MKD
		memset(receivedMsg,'\0', sizeof(receivedMsg)); //reintializing receivedMsg to \0 so that we can store the reply to send in this variable
		if(strlen(param)==0) { //checking if an argument is provided in the command sent by client because we need name of the directory to execute MKD command
			sprintf(receivedMsg, "Reply by server: No argument has been supplied, MKD command requires an argument\n"); //reply incase no argument
		}
		else{ //if argument is supplied
			flag=mkdir(param,0777); //making a directory with name provided by client
			if(flag==0){ //checking if directory is successfully created or not
				sprintf(receivedMsg, "Reply by server: 257 directory %s has been created\n", param);
			}
			else{ //incase it is not successfully created
				sprintf(receivedMsg, "Reply by server: 550 error while creating directory %s\n", param);
			}
		}
		//reinitializing to \0
		memset(cmdExt,'\0',sizeof(cmdExt));
		memset(param,'\0',sizeof(param));
	}
	else if(strcmp(cmdExt, "RMD")==0 || strcmp(cmdExt, "rmd")==0) { //checking if command entered by client is RMD
		
		memset(receivedMsg,'\0', sizeof(receivedMsg)); //reintializing receivedMsg to \0 so that we can store the reply to send in this variable
		if(strlen(param)==0) {//checking if an argument is provided in the command sent by client because we need name of the directory to execute RMD command
			sprintf(receivedMsg, "Reply by server: No argument has been supplied, RMD command requires an argument\n"); //reply incase no argument
		}
		else{ //if argument is supplied
			flag=rmdir(param); //removing the directory whose name is passed as parameter by client
			if(flag==0){ //incase directory successfully removed
			sprintf(receivedMsg, "Reply by server: 250 directory %s has been removed\n", param);
			}
			else{ //error while removing the directory
				sprintf(receivedMsg, "Reply by server: 550 error while removing directory %s\n", param);
			}
		}
		//reinitializing back to \0
		memset(cmdExt,'\0',sizeof(cmdExt));
		memset(param,'\0',sizeof(param));
	}
		else if(strcmp(cmdExt, "cwd")==0 || strcmp(cmdExt, "CWD")==0) { //check if command entered by client is CWD
		memset(receivedMsg, '\0', sizeof(receivedMsg)); //reintializing receivedMsg to \0 so that we can store the reply to send in this variable
		if(strlen(param)==0) {//checking if an argument is provided in the command sent by client because we need name of directory to change it to, using CWD command
			sprintf(receivedMsg, "Reply by server: No argument has been supplied, CWD command requires an argument\n"); //reply if no argument
		}
		else{// if argument is provided
			flag= chdir(param); //change directory to the argument provided as directory name
			getcwd(buffer,sizeof(buffer)); //getting current working directory after changing
			if(flag==0){ //if changing directory is done successfully
				sprintf(receivedMsg, "Reply by server: 250 directory changed to %s\n", buffer);
			}
			else { //error while changing directory
				sprintf(receivedMsg, "Reply by server: 550 not able to change directory\n");
			}
		}
		//reinitializing to \0
		memset(cmdExt,'\0',sizeof(cmdExt));
		memset(param, '\0', sizeof(param));
	}

	else if(strcmp(cmdExt, "cdup")==0 || strcmp(cmdExt, "CDUP")==0) { //checking if command entered by client is CDUP
		memset(receivedMsg, '\0', sizeof(receivedMsg));  //reintializing receivedMsg to \0 so that we can store the reply to send in this variable
			char * parent=".."; //variable to store parent directory 
			flag= chdir(parent); //changing to parent directory
			getcwd(buffer,sizeof(buffer)); //getting current working directory after changing
			if(flag==0){ //if changing directory is done successfully
				sprintf(receivedMsg, "Reply by server: 250 directory changed to %s\n", buffer);
			}
			else { //error while changing directory
			sprintf(receivedMsg, "Reply by server: 550 not able to change directory\n");
			}
		//reinitilizing back to \0
		memset(cmdExt,'\0',sizeof(cmdExt));
		memset(param, '\0', sizeof(param));
	}


	else if(strcmp(cmdExt, "stat")==0 || strcmp(cmdExt, "STAT")==0) {
		struct stat statistics; //structure to store stats of filename entered 
		memset(receivedMsg, '\0', sizeof(receivedMsg));  //reintializing receivedMsg to \0 so that we can store the reply to send in this variable
		memset(buffer, '\0', sizeof(buffer)); //reinitializing buffer to \0
		
		flag=stat(param,&statistics); //calling the stat command and storing statistics in a structure of type stat
		if(flag==0){ //if stat command was successfully called
			sprintf(receivedMsg,"Reply by server: 250 stat command executed!\n");
			//Concatenating all the information and stats to reply message
			sprintf(buffer, "Dev ID:  %ld\n", (int) statistics.st_dev);
			strcat(receivedMsg,buffer);
			sprintf(buffer, "Mode:  %08x\n", statistics.st_mode);
			strcat(receivedMsg,buffer);
			sprintf(buffer, "I-node number: %ld\n", (long) statistics.st_ino);
			strcat(receivedMsg,buffer);
			sprintf(buffer, "Links: %d\n", statistics.st_nlink);
			strcat(receivedMsg,buffer);
			sprintf(buffer, "UID: %d\n",(int) statistics.st_uid);
			strcat(receivedMsg,buffer);
			sprintf(buffer, "GID: %d\n",   (int) statistics.st_gid);
			strcat(receivedMsg,buffer);
			sprintf(buffer, "Input/Output block size: %ld bytes\n",(long) statistics.st_blksize);
			strcat(receivedMsg,buffer);
			sprintf(buffer, "Allocated blocks: %lld\n",(long long) statistics.st_blocks);
			strcat(receivedMsg,buffer);
			sprintf(buffer, "Size of file: %lld bytes\n",(long long) statistics.st_size);
			strcat(receivedMsg,buffer);
			sprintf(buffer, "Last file access: %s \n", ctime(&statistics.st_atime));
			strcat(receivedMsg,buffer);
			sprintf(buffer, "Last status change: %s \n", ctime(&statistics.st_ctime));
			strcat(receivedMsg,buffer);
			sprintf(buffer, "Last file modification:  %s \n ", ctime(&statistics.st_mtime));
			strcat(receivedMsg,buffer);
		}
		else{ //if stat command was not executed successfully
			sprintf(receivedMsg,"Reply by server: Error executing stat command\n");
		}
		
		//reinitializing back to \0
		memset(cmdExt,'\0',sizeof(cmdExt));
		memset(param, '\0', sizeof(param));
	}
  
	else if(strcmp(cmdExt, "dele")==0 || strcmp(cmdExt, "DELE")==0) { //checking if the command sent by client is DELE
		memset(receivedMsg,'\0', sizeof(receivedMsg)); //reintializing receivedMsg to \0 so that we can store the reply to send in this variable
		if(strlen(param)==0) { //checking if an argument is provided in the command sent by client because we need name of the file to delete using DELE command
			sprintf(receivedMsg, "Reply by server: No argument has been supplied, DELE command requires an argument\n"); //replace incase no argument
		}
		else{ //if argument is supplied
			flag=remove(param); //remove the file with filename passed as argument
			if(flag==0){ //if file deleted succcessfully
				sprintf(receivedMsg, "Reply by server: 250 file %s has been removed\n", param);
			}
			else{ //if there is an error while deleting the file
				sprintf(receivedMsg, "Reply by server: 550 error while removing file %s\n", param);
			}
		}
		//reinitializing back to \0
		memset(cmdExt,'\0',sizeof(cmdExt));
		memset(param,'\0',sizeof(param));
	}

	else if(strcmp(cmdExt, "rnfr")==0 || strcmp(cmdExt, "RNFR")==0) { //checking if the command entered by client is RNFR
		memset(receivedMsg,'\0', sizeof(receivedMsg));  //reintializing receivedMsg to \0 so that we can store the reply to send in this variable
		if(strlen(param)==0) { //checking if an argument is provided in the command sent by client because we need name of the file to be rename using RNFR command
			sprintf(receivedMsg, "Reply by server: No argument has been supplied, RNFR command requires an argument\n"); //reply if no argument provided
		}
		else{ //if argument is provided
			strcpy(renameFrom,param); //store the name of file to be renamed in variable renameFrom and now ask to client to send name of file to which it has to be renamed
			sprintf(receivedMsg, "Reply by server: 350, now enter RNTO command with the new name for file\n");
		}
		//reinitializing back to \0
		memset(cmdExt,'\0',sizeof(cmdExt));
		memset(param,'\0',sizeof(param));
	} 

	else if(strcmp(cmdExt, "rnto")==0 || strcmp(cmdExt, "RNTO")==0) { //checking if the command entered by client is RNTO
		memset(receivedMsg,'\0', sizeof(receivedMsg)); //reintializing receivedMsg to \0 so that we can store the reply to send in this variable
		if(strlen(param)==0) { //checking if an argument is provided in the command sent by client because we need new name of the file to which the file is renamed to using RNTO command
			sprintf(receivedMsg, "Reply by server: No argument has been supplied, RNTO command requires an argument\n"); //reply if no argument
		}
		else{ //incase argument is provided
			strcpy(renameTo,param); //store the name to which file has to be renamed in variable renameTo
			int check= rename(renameFrom,renameTo); //rename the file
			if(check==0){ //check if renamed successfully
				sprintf(receivedMsg, "Reply by server: 250, RNTO successfull\n");
				//reinitializing back to \0
				memset(renameFrom,'\0', sizeof(renameFrom));
				memset(renameTo,'\0', sizeof(renameTo));
			}
			else{ //error while renaming
				sprintf(receivedMsg, "Reply by server: 550, Could not rename the file\n");
			}

		}
		//reinitializing back to \0
		memset(cmdExt,'\0',sizeof(cmdExt));
		memset(param,'\0',sizeof(param));
	}


	else if(strcmp(cmdExt, "user")==0 || strcmp(cmdExt, "USER")==0) { //check if the command entered by client is USER
		char copyUserList[1024]; //variable to store a copy of auth list
		char * userTokens; //variable to store parts of auth list
		int checkUser=0; //variable to check if the user entered by client is present in the authorized user list 
		strcpy (copyUserList, userAuthList); //storing copy of authorized user in another variable
		userTokens= strtok(copyUserList, "\n"); //tokenizing and dividing into parts
		do { //loop to keep checking entered user by client in auth list one by one
		sscanf(userTokens, "%s %s", user, pass); //extracting user and pass from auth list token wise
		if(strcmp(param, user)==0) { //if user entered by client is present in auth list
			sprintf(receivedMsg, "Reply by server: 230 user exists, now enter password\n");
			checkUser=1; //setting user found flag to 1
			break;
		}
		userTokens= strtok(NULL, "\n");
		memset(receivedMsg, '\0', sizeof(receivedMsg));
		} while (userTokens!=NULL);
		if(checkUser==0){ //if user is not present in auth list
			sprintf(receivedMsg, "Reply by server: 331 user does not exist.\n"); 
		}
		
	}
  
	else if(strcmp(cmdExt, "pass")==0 || strcmp(cmdExt, "PASS")==0) { //checking if command entered by client is PASS
		memset(receivedMsg, '\0', sizeof(receivedMsg)); //reintializing receivedMsg to \0 so that we can store the reply to send in this variable
		if(pass[0]=='\0'){
			sprintf(receivedMsg, "Reply by server: 332 Unknown user\n"); //if user is unknown i.e not present in auth list
		}
		if(strcmp(param, pass)==0){ //if password entered by client matches
			sprintf(receivedMsg, "Reply by server: 230 password correct logged in");
		}
		else sprintf(receivedMsg, "Reply by server: 520 incorrect login"); //if it doesnt match
	}

	else if(strcmp(cmdExt, "abor")==0 || strcmp(cmdExt, "ABOR")==0) {//checking if command entered by client is ABOR
		memset(receivedMsg, '\0', sizeof(receivedMsg)); //reintializing receivedMsg to \0 so that we can store the reply to send in this variable
		strcpy(receivedMsg, "Reply by server: 226 aborting the server\n"); 
		sendMsg(ctrlSocket,receivedMsg,strlen(receivedMsg) + 1); //sending message back to client before aborting server
		abort(); //aborintg the server
	} 
	else if(strcmp(cmdExt, "quit")==0 || strcmp(cmdExt, "QUIT")==0) { //checking if command entered by client is QUIT
		memset(receivedMsg, '\0', sizeof(receivedMsg));//reintializing receivedMsg to \0 so that we can store the reply to send in this variable
		strcpy(receivedMsg, "Reply by server: 221 Goodbye\n"); //replying back to client before quitting server
	}

	else if(strcmp(cmdExt, "noop")==0 || strcmp(cmdExt, "NOOP")==0) { //checking if command entered by client is NOOP
		memset(receivedMsg, '\0', sizeof(receivedMsg));//reintializing receivedMsg to \0 so that we can store the reply to send in this variable
		strcpy(receivedMsg, "Reply by server: 200 Noop Successfull OK\n");
		//reinitializing back to \0
		memset(cmdExt,'\0',sizeof(cmdExt));
		memset(param,'\0',sizeof(param));
	}
  
	else if(strcmp("RETR", cmdExt)==0 || strcmp("retr", cmdExt)==0) { //checking if command entered by client is RETR
			FILE *f1; //variable to store file pointer
			int retrCmdSocket; //variable to store socker number 
			dataConnection("127.0.0.1", &retrCmdSocket); //calling dataconnection function to create socket and bind to entered address
			f1=fopen(param, "r");//opening file in read mode
			
			if(f1!=NULL) { //check if we are able to open file or not
			while(!feof(f1)) {
				fread(buffer, sizeof(char), sizeof(buffer), f1); //reading the bytes
				flag = sendMsg(retrCmdSocket, buffer, strlen(buffer)+1);//sending message back to client
				if(flag!=0) break;
			}
			fclose(f1);//closing file
			close(retrCmdSocket); //closing socket
			sprintf(receivedMsg, "Reply by server: RETR command executed successfully.");
			} else {
			sprintf(receivedMsg, "Reply by server: It doesnt exist in the directory");
			}
			fclose(f1);//closing file 
	}

    else if(strcmp("STOR", cmdExt)==0||strcmp("stor", cmdExt)==0) {//if command entered by client is STOR
        int storCmdSocket;//variable to store socket no.
        dataConnection("127.0.0.1", &storCmdSocket); //calling dataconnection function to create socket and bind to entered address
        FILE *createFile=fopen(param, "w");//opening file in write mode
        if(createFile==NULL) {//checking if file opened successfully or not
          while(1) {
            if(msgLen==0) break;
            flag=receiveMsg(storCmdSocket,buffer,sizeof(buffer), &msgLen); //receive message 
            fwrite(buffer, sizeof(char), msgLen, createFile); //writing to the file opened
            fflush(createFile);
            memset(buffer, '\0', sizeof(buffer)); //reinitializing to \0
          }
          close(storCmdSocket); //closing socket
          fclose(createFile); //closing file
        }
		sprintf(receivedMsg, "Reply by server: STOR command executed- new file has been created"); 
  	} 
	else { // Else case where the command entered by client matches none of the commands
		sprintf(receivedMsg, "Reply by server: Invalid input command\n"); 
		//reinitializing to \0
		memset(cmdExt,'\0',sizeof(cmdExt));
		memset(param, '\0', sizeof(param));
	}
	   
	flag=sendMsg(ctrlSocket,receivedMsg,strlen(receivedMsg) + 1);//sending the reply back to client after executing the command
	if(flag < 0)
	{
		break;  
	}
	} while(strcasecmp(inputCmd,"quit")!=0); //breaking out of infinite loop if command from client is quit
	close (ctrlSocket);  //closing control socket
	close(svSocket);  //closing server socket
	printf("Control connection between server client is closed \n");
	printf("Server exited \n");
	return (flag);
}

