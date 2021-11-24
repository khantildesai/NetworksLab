#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>

#define STDIN 0 //shity titty

#define COMMANDINPUTSIZE 301 //command line input size

#define MAX_NAME 31 //30 char max username
#define MAX_DATA 301 //300 max data (one line)

//macros for messages types
#define LOGIN 0
#define LO_ACK 1
#define LO_NACK 2
#define EXIT 3
#define JOIN 4
#define JN_ACK 5
#define JN_NACK 6
#define LEAVE_SESS 7
#define NEW_SESS 8
#define NS_ACK 9
#define MESSAGE 10
#define QUERY 11
#define QU_ACK 12
#define NEW_ACC 13
#define NEW_ACC_ACK 14
#define NEW_ACC_NACK 15

int serverFD;
char* myClientID[31] = {'\0'};

//keywords for commands
const char login[] = "/login";
const char logout[] = "/logout";
const char joinsession[] = "/joinsession";
const char leavesession[] = "/leavesession";
const char createsession[] = "/createsession";
const char list[] = "/list";
const char quit[] = "/quit";

//delimeter for parsing command
const char delim[] = " ";

//message struct for message format
typedef struct message {
	unsigned int type;
	unsigned int size;
	unsigned char source[MAX_NAME];
	unsigned char data[MAX_DATA];
} message;

//len of serialized message struct
int serializedMessageLen = 2*sizeof(unsigned int) + (MAX_NAME + MAX_DATA)*sizeof(unsigned char);

//global variables to keep track of client data
int loggedIn = -1;
int inSession = -1;

//check if input is valid
int validInput(char* command, char* totalCommand);

//login into server script
int Login(char* totalCommand);

//serialize messages function
void serialize(message messagePacket, char* serialArr);

//deserializing message function
message deSerialize(char* serialArr);

//connect to server function
int connectToServer(int portNum);

//make login struct
int makeLoginMessage(char* totalCommand);

//make logout struct
int makeLogoutMessage(char* totalCommand);

//make join struct
int makeJoinSessionMessage(char* totalCommand);

//make leave struct
int makeLeaveSessionMessage(char* totalCommand);

//make create struct
int makeCreateSessionMessage(char* totalCommand);

//make list struct
int makeQueryMessage(char* totalCommand);

//make message struct
int makeMessage(char* totalCommand);

int main(void){
	//int FileDescriptor = connectToServer(5000);

	fd_set all_sockets, prepared_sockets;

	FD_ZERO(&all_sockets);
	FD_SET(STDIN, &all_sockets); // only STDIN at first
	//FD_SET(FileDescriptor, &readfds);

	while(1){// main loop
		//prepared_sockets to protect all_sockets from destructive select
		prepared_sockets = all_sockets;

		//buffers
		char command_buffer[COMMANDINPUTSIZE], input_buffer[COMMANDINPUTSIZE], message_buffer[COMMANDINPUTSIZE];

		//Clean buffers
		memset(input_buffer, '\0', COMMANDINPUTSIZE);
		memset(command_buffer, '\0', COMMANDINPUTSIZE);
		memset(message_buffer, '\0', COMMANDINPUTSIZE);

		select(FD_SETSIZE, &prepared_sockets, NULL, NULL, NULL);

		if(FD_ISSET(STDIN, &prepared_sockets)){
			//there was keyboard input
			fgets(input_buffer, COMMANDINPUTSIZE, stdin); //get input
			memcpy(message_buffer, input_buffer, COMMANDINPUTSIZE);

			char *firstCommand = strtok(input_buffer, delim); //get first command

			if (validInput(firstCommand, input_buffer) == 0){ //check if valid
				//printf("its valid\n");
			}
			else {
				printf("not valid\n");
			}

			memset(input_buffer, '\0', COMMANDINPUTSIZE); //reset message buffer
		}
		for(int iter = 0; iter < FD_SETSIZE; iter++){
            if (FD_ISSET(iter, &prepared_sockets)){
                if(iter != 0){
					//Creating the buffers right here
					char client_buffer[COMMANDINPUTSIZE];

					//Clean buffers
					memset(client_buffer, '\0', sizeof(client_buffer));

					int res = recv(iter, client_buffer, sizeof(client_buffer), 0);

					if(res == 0){
						close(iter);
						FD_CLR(iter, &all_sockets);
						exit(0);
					}
					else {
						//deal with packet from server
						message serverMessage = deSerialize(client_buffer);
						//int ok = handleServerMessage(serverMessage);
					}

					printf("%s\n", client_buffer);
					memset(client_buffer, '\0', sizeof(client_buffer));
                }
            }
		}
    }
}

int validInput(char* command, char* totalCommand){
	//check if input is a command and if globals variables indicate that the command is valid
	if (strcmp(command, login) == 0){
		if (loggedIn == -1){
			int shit = makeLoginMessage(totalCommand);
			if (serverFD < 0) {printf("failed login\n"); return -1; }
			serverFD = shit; return 0;}
		else {return -1;}}
	else if (strcmp(command, logout) == 0){
		if (loggedIn == 1){ 
			if (makeLogoutMessage(totalCommand) != -15){ return -1; }
			return 0;}
		else {return -1;}}
	else if (strcmp(command, joinsession) == 0){
		if ((loggedIn == 1) & inSession == -1){ 
			if (makeJoinSessionMessage(totalCommand) != 0){ return -1; }
			return 0; }
		else { return -1; }}
	else if (strcmp(command, leavesession) == 0){
		if ((loggedIn == 1) & inSession == 1){ 
			if (makeLeaveSessionMessage(totalCommand) != 0){ return -1; }
			return 0; }
		else {return -1; }}
	else if (strcmp(command, createsession) == 0){
		if ((loggedIn == 1) & inSession == -1){ 
			if (makeCreateSessionMessage(totalCommand) != 0){ return -1; }
			return 0; }
		else { return -1; }}
	else if (strcmp(command, list) == 0){
		if ((loggedIn == 1) & inSession == 1){ 
			if (makeQueryMessage(totalCommand) != 0){ return -1; }
			return 0; }
		else { return -1; }}
	else if (strcmp(command, quit) == 0){
		if ((loggedIn == 1) & inSession == 1){ 
			exit(0);
			return 0; }
		else { return -1; }}
	else { 
		if (makeMessage(totalCommand) != 0){ return -1; }
		return 0; }
}

void serialize(message messagePacket, char* serialArr){
	memcpy(serialArr, &messagePacket.type, sizeof(unsigned int)); //type
	memcpy(serialArr + sizeof(unsigned int), &(messagePacket.size), sizeof(unsigned int)); //size
	memcpy(serialArr + 2*sizeof(unsigned int), &(messagePacket.source), MAX_NAME*sizeof(unsigned char)); //source
	memcpy(serialArr + 2*sizeof(unsigned int) + MAX_NAME*sizeof(unsigned char), &(messagePacket.data), messagePacket.size); //data
}

message deSerialize(char* serialArr){
	//packet to return
	message toReturn;

	//type
	memcpy(&toReturn.type, serialArr, sizeof(unsigned int));

	//size
	memcpy(&toReturn.size, serialArr + sizeof(unsigned int), sizeof(unsigned int));

	//source
	memcpy(&toReturn.source, serialArr + 2*sizeof(unsigned int), MAX_NAME*sizeof(unsigned int));

	//data
	memcpy(&toReturn.data, serialArr + 2*sizeof(unsigned int) + MAX_NAME*sizeof(unsigned char), MAX_DATA*sizeof(unsigned char));

	return toReturn;
}

int connectToServer(int portNum){
	//socket for communicating nini
	int FileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
	if(FileDescriptor < 0){
		printf("You messed up in sockeet\n"); return -1;}
	//printf("Socket is looking good\n");

	struct sockaddr_in server_address;

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(portNum);
	server_address.sin_addr.s_addr = inet_addr("128.100.13.176");

	//Send request nini to server
	if(connect(FileDescriptor, (struct sockaddr *) &server_address, sizeof(server_address)) < 0){
		printf("the nini could not connect\n");
		return -1;
	}
	printf("Ready!\n");
	return FileDescriptor;
}

int makeLoginMessage(char* totalCommand){
	int valid = 0; //0 = valid

	//get login parameters
	char *clientID = strtok(NULL, delim);
	char *password = strtok(NULL, delim);
	char *serverIP = strtok(NULL, delim);
	char *serverPort = strtok(NULL, delim);

	//ensure proper parameters
	if (clientID == NULL){printf("Invalid Login Parameters!"); valid = -1;}
	if (password == NULL){printf("Invalid Login Parameters!"); valid = -1;}
	if (serverIP == NULL){printf("Invalid Login Parameters!"); valid = -1;}
	if (serverPort == NULL){printf("Invalid Login Parameters!"); valid = -1;}

	//get port num
	int portNum = atoi(serverPort);

	//make login message and 
	int loginFD = connectToServer(portNum);

	//create Message Packet struct
	if (valid != 0){
		return -1;
	}
	else {
		//create message packet struct
		message packetMessage;
		packetMessage.type = LOGIN;
		memcpy(packetMessage.source, clientID, strlen(clientID));
		memcpy(packetMessage.data, password, strlen(password));
		packetMessage.size = strlen(packetMessage.data);

		//serialize
		char serializedMessage[400] = {"\0"};
		serialize(packetMessage, serializedMessage);

		//Sending the message nini to server
		if(send(loginFD, serializedMessage, sizeof(serializedMessage), 0) < 0){
			printf("There was an error in sending\n");
			return -1;
		}
		printf("sent message to server\n");
		memcpy(myClientID, clientID, 31);
		return loginFD;
	}
}

//make logout struct
int makeLogoutMessage(char* totalCommand){
	close(serverFD);
	return -15;
}

//make logout struct
int makeJoinSessionMessage(char* totalCommand){
	int valid = 0; //0 = valid

	//get login parameters
	char *sessionID = strtok(NULL, delim);

	//ensure proper parameters
	if (sessionID == NULL){printf("Invalid Login Parameters!"); valid = -1;}

	//create Message Packet struct
	if (valid != 0){
		return -1;
	}
	else {
		//create message packet struct
		message packetMessage;
		packetMessage.type = JOIN;
		memcpy(packetMessage.source, myClientID, 31);
		memcpy(packetMessage.data, sessionID, strlen(sessionID));
		packetMessage.size = strlen(packetMessage.data);

		//serialize
		char serializedMessage[400] = {"\0"};
		serialize(packetMessage, serializedMessage);

		//Sending the message nini to server
		if(send(serverFD, serializedMessage, sizeof(serializedMessage), 0) < 0){
			printf("There was an error in sending\n");
			return -1;
		}
		printf("sent joinsession message to server\n");
		return 0;
	}
}

//make logout struct
int makeCreateSessionMessage(char* totalCommand){
	int valid = 0; //0 = valid

	//get login parameters
	char *sessionID = strtok(NULL, delim);

	//ensure proper parameters
	if (sessionID == NULL){printf("Invalid Login Parameters!"); valid = -1;}

	//create Message Packet struct
	if (valid != 0){
		return -1;
	}
	else {
		//create message packet struct
		message packetMessage;
		packetMessage.type = NEW_SESS;
		memcpy(packetMessage.source, myClientID, 31);
		memcpy(packetMessage.data, sessionID, strlen(sessionID));
		packetMessage.size = strlen(packetMessage.data);

		//serialize
		char serializedMessage[400] = {"\0"};
		serialize(packetMessage, serializedMessage);

		//Sending the message nini to server
		if(send(serverFD, serializedMessage, sizeof(serializedMessage), 0) < 0){
			printf("There was an error in sending\n");
			return -1;
		}
		printf("sent joinsession message to server\n");
		return 0;
	}
}


//make logout struct
int makeLeaveSessionMessage(char* totalCommand){
	//create message packet struct
	message packetMessage;
	packetMessage.type = LEAVE_SESS;
	memcpy(packetMessage.source, myClientID, 31);
	packetMessage.size = 0;

	//serialize
	char serializedMessage[400] = {"\0"};
	serialize(packetMessage, serializedMessage);

	//Sending the message nini to server
	if(send(serverFD, serializedMessage, sizeof(serializedMessage), 0) < 0){
		printf("There was an error in sending\n");
		return -1;
	}
	printf("sent leave session message to server\n");
	return 0;
}

//make logout struct
int makeQueryMessage(char* totalCommand){
	//create message packet struct
	message packetMessage;
	packetMessage.type = QUERY;
	memcpy(packetMessage.source, myClientID, 31);
	packetMessage.size = 0;

	//serialize
	char serializedMessage[400] = {"\0"};
	serialize(packetMessage, serializedMessage);

	//Sending the message nini to server
	if(send(serverFD, serializedMessage, sizeof(serializedMessage), 0) < 0){
		printf("There was an error in sending\n");
		return -1;
	}
	printf("sent leave session message to server\n");
	return 0;
}

int makeMessage(char* totalCommand){
	//create message packet struct
	message packetMessage;
	packetMessage.type = MESSAGE;
	memcpy(packetMessage.source, myClientID, 31);
	memcpy(packetMessage.data, totalCommand, 301);
	packetMessage.size = 301;

	//serialize
	char serializedMessage[400] = {"\0"};
	serialize(packetMessage, serializedMessage);

	//Sending the message nini to server
	if(send(serverFD, serializedMessage, sizeof(serializedMessage), 0) < 0){
		printf("There was an error in sending\n");
		return -1;
	}
	printf("sent joinsession message to server\n");
	return 0;
}