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

int validInput(char* command);

int main(void){
    //int serverPort = atoi(argv[2]);
    
    //char fname[300];
    char commandInput[COMMANDINPUTSIZE];
    memset(commandInput, 0, COMMANDINPUTSIZE);

    while(1 == 1){// main loop
	printf("enter command:\n");
    	fgets(commandInput, COMMANDINPUTSIZE, stdin);
	
	const char *token = strtok(commandInput, delim);
	char testToken[100];
	strcpy(testToken, token);	


	printf("the string is: %s\n", token);

	if (validInput(testToken) == 0){
	    printf("its valid\n");
	}
	else {
	    printf("not valid\n");
	}

        memset(commandInput, 0, COMMANDINPUTSIZE);
    }
}

int validInput(char* command){
    printf("command is: %s\n", command);
    //const char test[] = "/login";
    if (strcmp(command, login) == 0){
    	printf("recognized\n");
	if (loggedIn == -1){
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
    else{
       return -1;
    }
}
    
