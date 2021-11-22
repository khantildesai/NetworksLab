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
#define COMMANDINPUTSIZE 2000

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
	int FileDescriptor = socket(AF_INET, SOCK_STREAM, 0);

	char server_buffer[COMMANDINPUTSIZE], client_buffer[COMMANDINPUTSIZE];

	//Clean buffers
	memset(server_buffer, '\0', sizeof(server_buffer));
	memset(client_buffer, '\0', sizeof(client_buffer));

	if(FileDescriptor < 0){
		printf("You messed up in sockeet\n");
		return -1;
	}

	printf("Socket is looking good\n");

	struct sockaddr_in server_address;

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(2000); //This is is super hardcoded REMEMBER TO CHANGE OTHERWISE PROBLEMS ALWAYS
	server_address.sin_addr.s_addr = inet_addr("128.100.13.171");

	//Send request nini to server
	if(connect(FileDescriptor, (struct sockaddr *) &server_address, sizeof(server_address)) < 0){
		printf("the nini could not connect\n");
		return -1;
	}
	printf("connected successfully\n");

	while(1){// main loop
		//printf("enter command:\n");
		fgets(client_buffer, COMMANDINPUTSIZE, stdin);
		//Sending the message nini to server
		if(send(FileDescriptor, client_buffer, strlen(client_buffer), 0) < 0){
			printf("There was an error in sending");
			return -1;
		}
		
		
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
    
