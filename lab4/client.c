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
#define COMMANDINPUTSIZE 300

#define MAX_NAME 31
#define MAX_DATA 300

struct message {
    unsigned int type;
    unsigned int size;
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
};

int loggedIn = -1;
int inSession = -1;

const char login[] = "/login";
const char logout[] = "/logout";
const char joinsession[] = "/joinsession";
const char leavesession[] = "/leavesession";
const char createsession[] = "/createsession";
const char list[] = "/list";
const char quit[] = "/quit";

const char delim[] = " ";

int validInput(char* command, char* totalCommand);

int Login(char* totalCommand);

int main(void){
    //int serverPort = atoi(argv[2]);
    
    char fname[300];
    char commandInput[COMMANDINPUTSIZE];
    memset(commandInput, 0, COMMANDINPUTSIZE);

    while(1){// main loop
	    printf("enter command:\n");
    	fgets(commandInput, COMMANDINPUTSIZE, stdin);
	
	    char *token = strtok(commandInput, delim);

	    if (validInput(token, commandInput) == 0){
	        printf("its valid\n");
	    }
	    else {
	        printf("not valid\n");
	    }

        memset(commandInput, 0, COMMANDINPUTSIZE);
    }

    int FileDescriptor = socket(AF_INET, SOCK_STREAM, 0);

    char *msg = "lOGIN: Please";
}

int validInput(char* command, char* totalCommand){
    if (strcmp(command, login) == 0){
	    if (loggedIn == -1){
            Login(totalCommand);
	        return 0;
	    }
	    else {
	        return -1;
	    }
    }
    else if (strcmp(command, logout) == 0){
        if (loggedIn == 1){
	        return 0;
	    }
	    else {
	        return -1;
	    }
    }
    else if (strcmp(command, joinsession) == 0){
        if ((loggedIn == 1) & inSession == -1){
	        return 0;
	    }
	    else {
	        return -1;
	    }
    }
    else if (strcmp(command, leavesession) == 0){
        if ((loggedIn == 1) & inSession == 1){
	        return 0;
	    }
	    else {
	        return -1;
	    }
    }
    else if (strcmp(command, createsession) == 0){
        if ((loggedIn == 1) & inSession == -1){
	        return 0;
	    }
	    else {
	        return -1;
	    }
    }
    else if (strcmp(command, list) == 0){
        if ((loggedIn == 1) & inSession == 1){
	        return 0;
	    }
	    else {
	        return -1;
	    }
    }
    else if (strcmp(command, quit) == 0){
        if ((loggedIn == 1) & inSession == 1){
	        return 0;
	    }
	    else {
	        return -1;
	    }
    }
    else{
       return -1;
    }
}

int Login(char* totalCommand){
    char *clientID = strtok(NULL, delim);
    char *password = strtok(NULL, delim);
    char *serverIP = strtok(NULL, delim);
    char *serverPort = strtok(NULL, delim);
    
}
