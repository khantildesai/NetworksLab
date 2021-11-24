#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h>

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

#define COMMANDINPUTSIZE 301 //command line input size

#define MAX_NAME 31 //30 char max username
#define MAX_DATA 301 //300 max data (one line)

//message struct for message format
typedef struct message {
	unsigned int type;
	unsigned int size;
	unsigned char source[MAX_NAME];
	unsigned char data[MAX_DATA];
} message;

//user information struct
typedef struct user{
    char clientID[MAX_NAME];
    char password[MAX_NAME];
    char sessionID[MAX_NAME];
} user;

user list_of_users[4] = {{"Joel", "0000", "general"}, {"Khantil", "1234", "general"}, {"Jane", "1111", "general"}, {"Natalie", "Khantil", "general"}};

//keywords for commands
const char login[] = "/login";
const char logout[] = "/logout";
const char joinsession[] = "/joinsession";
const char leavesession[] = "/leavesession";
const char createsession[] = "/createsession";
const char list[] = "/list";
const char quit[] = "/quit";

int acceptConnect(int listeningFD);
int setup_listen(int portNum);
int handleNini(int clientFD);

//serialize messages function
void serialize(message messagePacket, char* serialArr);

//deserializing message function
message deSerialize(char* serialArr);

int main(int argc, char *argv[]){

    if (argc != 2){
        return 1;
    }
    int portNum = atoi(argv[1]);

    int FileDescriptor = setup_listen(portNum);

    fd_set all_sockets, prepared_sockets;

    //initialize sets
    FD_ZERO(&all_sockets);
    FD_SET(FileDescriptor, &all_sockets);
    int testflag = 0;
    while(1){
        prepared_sockets = all_sockets;

        if (select(FD_SETSIZE, &prepared_sockets, NULL, NULL, NULL) < 0){
            perror("select done goofed!");
            exit(-1);
        }

        for(int iter = 0; iter < FD_SETSIZE; iter++){
            if (FD_ISSET(iter, &prepared_sockets)){
                if (iter == FileDescriptor){
                    //accepting this connection request
                    int new_con = acceptConnect(FileDescriptor);
                    FD_SET(new_con, &all_sockets);
                } else {
                    if (handleNini(iter) == -1){
                        close(iter);
                        FD_CLR(iter, &all_sockets);
                    }
                }
            }
        }
    }
}

int acceptConnect(int listeningFD){
    //Address information of peer struct
    struct sockaddr_storage storageAddress;
    struct sockaddr_storage *storageAddressPtr;
    storageAddressPtr = &storageAddress;
    socklen_t addressSize = sizeof(storageAddress);
    socklen_t *addressSizePtr = &addressSize;

    int new_socket;

    if(!(listen(listeningFD, 7))){
        new_socket = accept(listeningFD, (struct sockaddr *)storageAddressPtr, addressSizePtr);
    }
    return new_socket;
}

int setup_listen(int portNum){
    int FileDescriptor = socket(AF_INET, SOCK_STREAM, 0);

    //making sockaddr_in struct for listening socket
    struct in_addr addrStruct;
    addrStruct.s_addr = htonl(INADDR_ANY);
    struct sockaddr_in socketAddr;
    socketAddr.sin_family = AF_INET;
    socketAddr.sin_port = htons(portNum);
    socketAddr.sin_addr = addrStruct;
    
    //pointer to struct
    struct sockaddr_in *addrPtr;
    addrPtr = &socketAddr;

    //bind check
    int bindSucc = bind(FileDescriptor, (const struct sockaddr *)addrPtr, sizeof(struct sockaddr));
    if(bindSucc == -1){
        printf("bind failed\n");
    }

    return FileDescriptor;  
}

int handleNini(int clientFD){
    //Creating the buffers right here
    char client_buffer[COMMANDINPUTSIZE];
    char server_response[COMMANDINPUTSIZE] = "login success";
    const char check[] = "login";
    const char delim[] = " ";
    char *firstword;

    int numBytesRecv;

    numBytesRecv = recv(clientFD, client_buffer, sizeof(client_buffer), 0);
    if (numBytesRecv == 0){
        printf("connect broken :(\n");
        return -1;
    }
    printf("%s", client_buffer);
    firstword = strtok(client_buffer, delim);
    if(strcmp(firstword, check) == 0){
        if(send(clientFD, server_response, strlen(server_response), 0) < 0){
            printf("The server failed at responding.");
        }
    }
    memset(client_buffer, '\0', sizeof(client_buffer));
    memset(server_response, '\0', sizeof(server_response));
}

void serialize(message messagePacket, char* serialArr){
	memcpy(serialArr, &messagePacket.type, sizeof(unsigned int)); //type
	memcpy(serialArr + sizeof(unsigned int), &(messagePacket.size), sizeof(unsigned int)); //size
	memcpy(serialArr + 2*sizeof(unsigned int), &(messagePacket.source), MAX_NAME*sizeof(unsigned char)); //source
	memcpy(serialArr + 2*sizeof(unsigned int) + MAX_NAME*sizeof(unsigned char), &(messagePacket.data), MAX_DATA*sizeof(unsigned char)); //data
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
}