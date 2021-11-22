#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <ctype.h>

#define POLLING 32
#define COMMANDINPUTSIZE 300
#define LISTEN_BACKLOG 50
//name and data macro
#define MAX_NAME 31
#define MAX_DATA 301

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

//global variables to keep track of cilent state
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
message deSerialize(char* serialArr);

//main
int main(int argc, char *argv[]){
    if (argc != 2){ //if less/more than 1 arg given; return
        return 1;
    }
    int portNum = atoi(argv[1]);

    int listeningFD = socket(AF_INET, SOCK_STREAM, 0);
    printf("starting to listen...\n");
    if (listen(listeningFD, LISTEN_BACKLOG) == -1){
        printf("ERROR: LISTEN FAILED!\n"); return -1;
    }
    
    //structs for storing address of client to connect
    struct sockaddr_storage storageAddress;
    struct sockaddr_storage *storageAddressPtr = &storageAddress;
    socklen_t addressSize = sizeof(storageAddress);
    socklen_t *addressSizePtr = &addressSize;

    if (accept(listeningFD,(struct sockaddr*) storageAddressPtr, addressSizePtr) == -1){
        printf("ERROR: ACCEPT FAILED!\n"); return -1;
    }

}

message deSerialize(char* serialArr){
    //packet to return
    message toReturn;
    
    //type
    memcpy(&toReturn.type, serialArr, sizeof(unsigned int));
    
    //size
    memcpy(&toReturn.size, serialArr + sizeof(unsigned int), sizeof(unsigned int));
    
    //source
    memcpy(&toReturn.source, serialArr + 2*(sizeof(unsigned int)), MAX_NAME*sizeof(unsigned char));
    
    //data
    memcpy(&toReturn.data, serialArr + 2*(sizeof(unsigned int)) + MAX_NAME*sizeof(unsigned char), MAX_DATA*sizeof(unsigned char));
    
    return toReturn;
}
