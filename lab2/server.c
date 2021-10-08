#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <ctype.h>

typedef struct packet {
    unsigned int total_frag;    //total number of fragments of the file.
    unsigned int frag_no;    //sequenze number
    unsigned int size;    //size of data in range from 0 to 1000
    char* filename;
    char filedata[1000];
} packet;

packet deSerialize(char* serialArr);

int main(int argc, char *argv[]){
    if (argc != 2){ //if less/more than 1 arg given; return
        return 1;
    }
    int portNum = atoi(argv[1]);

    int FileDescriptor = socket(AF_INET, SOCK_DGRAM, 0);

    //making sockaddr_in struct
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

    //using the recvfrom function
    char buffer[10000];
    struct sockaddr_storage storageAddress;
    struct sockaddr_storage *storageAddressPtr;
    storageAddressPtr = &storageAddress;
    socklen_t addressSize = sizeof(storageAddress);
    socklen_t *addressSizePtr = &addressSize;
    printf("Server receiving on port %d.\n", portNum);

    while(1){
        recvfrom(FileDescriptor, buffer, 200, 0, (struct sockaddr *) storageAddressPtr, addressSizePtr);
        //checking if it is yes or no to send to the client
        if(strcmp(buffer,"ftp") == 0){
	    printf("recieved message, sending reply!\n");
            sendto(FileDescriptor, "yes", strlen("yes"), 0, (struct sockaddr *) storageAddressPtr, sizeof(storageAddress));
        }
        else{
	    printf("should be colon %c\n", buffer[9]);
            packet curr = deSerialize(buffer);
	    printf("should be colon: %c", buffer[15]);
	    sendto(FileDescriptor, "no", strlen("no"), 0, (struct sockaddr *) storageAddressPtr, sizeof(storageAddress));
        }

    }
}

packet deSerialize(char* serialArr){
    //packet to return
    packet toReturn;
    
    //total_frag
    memcpy(&toReturn.total_frag, serialArr, 4);
    printf("copied total_frag\n");
    
    //frag_no
    memcpy(&toReturn.frag_no, serialArr + 5, 4);
    printf("copied frag_no\n");
    
    //size
    memcpy(&toReturn.size, serialArr + 10, 4);
    printf("copied size\n");
    
    //filename
    //char* fileNameInput = strtok(serialArr + 15, ":");
    //int fileNameSize = strlen(fileNameInput);
    //char* newfilename = malloc(fileNameSize);
    memcpy(&toReturn.filename, serialArr + 15, 11);
    printf("copied filename\n");
    
    //filedata
    //char* filedataPtr = strtok(serialArr + 27, "\0");
    memcpy(&toReturn.filedata, serialArr + 27, 1000);
    printf("copied filedata\n");

    //toReturn.total_frag = strtok(serialArr, ':');
    //toReturn.frag_no = strtok(NULL, ':');
    //toReturn.size = strtok(NULL, ':');
    //toReturn.filename = strtok(NULL, ':');
    //toReturn.filedata = strtok(NULL, '\0');
    return toReturn;
}

