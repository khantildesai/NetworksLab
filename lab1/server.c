#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <ctype.h>

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
    char buffer[200];
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
            sendto(FileDescriptor, "yes", strlen("yes"), 0, (struct sockaddr *) storageAddressPtr, sizeof(storageAddress));
        }
        else{
            sendto(FileDescriptor, "no", strlen("no"), 0, (struct sockaddr *) storageAddressPtr, sizeof(storageAddress));
        }

    }
    // recvfrom(FileDescriptor, buffer, 200, 0, (struct sockaddr *) storageAddressPtr, addressSizePtr);

    // //checking if it is yes or no to send to the client
    // if(strcmp(buffer,"ftp") == 0){
    //     sendto(FileDescriptor, "yes", strlen("yes"), 0, (struct sockaddr *) storageAddressPtr, sizeof(storageAddress));
    // }
    // else{
    //     sendto(FileDescriptor, "no", strlen("no"), 0, (struct sockaddr *) storageAddressPtr, sizeof(storageAddress));
    // }
}
