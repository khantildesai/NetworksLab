#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>

#define POLLING 32
#define COMMANDINPUTSIZE 300 //buffer size of the input line

#define MAX_NAME 31 //30 char max username
#define MAX_DATA 301 //300 char max data (one line)

//macros for message types
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

//message struct to be used for tcp messages
typedef struct message {
    unsigned int type;
    unsigned int size;
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
} message;
int serializedMessageLen = 2*sizeof(unsigned int) + (MAX_NAME + MAX_DATA)*sizeof(unsigned char);

//global variables to keep track of client state
int loggedIn = -1;
int inSession = -1;

//global variables to see if command keyword entered or not
const char login[] = "/login";
const char logout[] = "/logout";
const char joinsession[] = "/joinsession";
const char leavesession[] = "/leavesession";
const char createsession[] = "/createsession";
const char list[] = "/list";
const char quit[] = "/quit";

//delimeter to parse input
const char delim[] = " ";

//check if input is valid
int validInput(char* command, char* totalCommand);

//login into server script
int Login(char* totalCommand);

//serialize messages function
void serialize(message messagePacket, char* serialArr);

//main
int main(void){
    char commandInput[COMMANDINPUTSIZE]; //input buffer
    memset(commandInput, 0, COMMANDINPUTSIZE);//set input buffer to all 0

    while(1){// main loop
	printf("enter command:\n");
    	fgets(commandInput, COMMANDINPUTSIZE, stdin); //get input
	
	char *firstCommand = strtok(commandInput, delim); //get first command

	if (validInput(firstCommand, commandInput) == 0){ //check if valid
	    printf("its valid\n");
	}
	else {
	    printf("not valid\n");
	}

        memset(commandInput, 0, COMMANDINPUTSIZE); //reset message buffer
    }

    int FileDescriptor = socket(AF_INET, SOCK_STREAM, 0);

    char *msg = "lOGIN: Please";
}

int validInput(char* command, char* totalCommand){
    //check if input is a command and if globals variables indicate that the command is valid
    if (strcmp(command, login) == 0){
	if (loggedIn == -1){ Login(totalCommand); return 0;}
	else { return -1; }}
    else if (strcmp(command, logout) == 0){
        if (loggedIn == 1){ return 0; }
        else { return -1; }}
    else if (strcmp(command, joinsession) == 0){
        if ((loggedIn == 1) & inSession == -1){ return 0; }
	else { return -1; }}
    else if (strcmp(command, leavesession) == 0){
        if ((loggedIn == 1) & inSession == 1){ return 0; }
	else { return -1; }}
    else if (strcmp(command, createsession) == 0){
        if ((loggedIn == 1) & inSession == -1){ return 0; }
	else { return -1; }}
    else if (strcmp(command, list) == 0){
        if ((loggedIn == 1) & inSession == 1){ return 0; }
        else { return -1; }}
    else if (strcmp(command, quit) == 0){
        if ((loggedIn == 1) & inSession == 1){ return 0; }
	else { return -1; }}
    else{ return -1; }
}

int Login(char* totalCommand){
    //get login parameters
    char *clientID = strtok(NULL, delim);
    char *password = strtok(NULL, delim);
    char *serverIP = strtok(NULL, delim);
    char *serverPort = strtok(NULL, delim);
    
    //ensure proper parameters
    if (clientID == NULL){printf("Invalid Login Parameters!"); return -1;}
    if (password == NULL){printf("Invalid Login Parameters!"); return -1;}
    if (serverIP == NULL){printf("Invalid Login Parameters!"); return -1;}
    if (serverPort == NULL){printf("Invalid Login Parameters!"); return -1;}
    
    //
    int portNum = atoi(serverPort);
    printf("the port number is %d\n", portNum);

    //creating socket and binding
    struct sockaddr_in login_addr;
    int loginFD;
    
    login_addr.sin_family = AF_INET;
    login_addr.sin_port = htons(portNum);
    inet_aton(serverIP, &login_addr.sin_addr);
    //login_addr.sin_addr.s_addr = inet_addr(serverIP);

    loginFD = socket(AF_INET,SOCK_STREAM, 0);
    if (loginFD == -1){printf("ERROR: Socket Failed!\n"); return -1;}
    int connectSucc = connect(loginFD, (struct sockaddr*) &login_addr, sizeof(login_addr));
    if (connectSucc != 0){printf("ERROR: connection error!\n");}

    //make login message and find its length
    char loginMessage[65];
    strcat(loginMessage, clientID);
    int index = strlen(clientID);
    loginMessage[index] = ':';
    strcat(loginMessage + index + 1, password);
    int loginMessageLen = strlen(clientID) + 1 + strlen(password);

    //create Message Packet struct
    message packetMessage;
    packetMessage.type = LOGIN;
    packetMessage.size = loginMessageLen;
    memcpy(packetMessage.source, clientID, strlen(clientID));
    memcpy(packetMessage.source, loginMessage, loginMessageLen);
    
    //serialize message
    char messageArr[serializedMessageLen];
    serialize(packetMessage, messageArr);
    
    //send login Request
}

void serialize(message messagePacket, char* serialArr){
    memcpy(serialArr, &(messagePacket.type), sizeof(unsigned int));
    memcpy(serialArr + sizeof(unsigned int), &(messagePacket.size), sizeof(unsigned int));
    memcpy(serialArr + 2*sizeof(unsigned int), &(messagePacket.source), MAX_NAME*sizeof(unsigned char));
    memcpy(serialArr + 2*sizeof(unsigned int) + MAX_NAME*sizeof(unsigned char), &(messagePacket.data), MAX_DATA*sizeof(unsigned char));
}
