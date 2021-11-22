#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <ctype.h>

#define COMMANDINPUTSIZE 2000

struct message{
    unsigned int type;
    unsigned int size;
    unsigned char source[31];
    unsigned char data[301];
};
int main(int argc, char *argv[]){
    if (argc != 2){
        return 1;
    }
    int portNum = atoi(argv[1]);
    printf("%d\n", portNum);

    //Creating the buffers right here
    char server_buffer[COMMANDINPUTSIZE], client_buffer[COMMANDINPUTSIZE];

	//Clean buffers
	memset(server_buffer, '\0', sizeof(server_buffer));
	memset(client_buffer, '\0', sizeof(client_buffer));


    int FileDescriptor = socket(AF_INET, SOCK_STREAM, 0);

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

    //Address information of peer struct
    struct sockaddr_storage storageAddress;
    struct sockaddr_storage *storageAddressPtr;
    storageAddressPtr = &storageAddress;
    socklen_t addressSize = sizeof(storageAddress);
    socklen_t *addressSizePtr = &addressSize;

    int new_socket;

    if(!(listen(FileDescriptor, 7))){
        new_socket = accept(FileDescriptor, (struct sockaddr *)storageAddressPtr, addressSizePtr);
    }
    printf("new_socket\n");  //Might want to remove this later on.

    while(1){
        if(recv(new_socket, client_buffer, sizeof(client_buffer), 0) < 0){
            printf("Ya I couldn't receive\n");
            return -1;
        }
        printf("%s", client_buffer);
        memset(client_buffer, '\0', sizeof(client_buffer));
    }
    


}
