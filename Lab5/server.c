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

#define COMMANDINPUTSIZE 2000

struct message{
    unsigned int type;
    unsigned int size;
    unsigned char source[31];
    unsigned char data[301];
};

int acceptConnect(int listeningFD);
int setup_listen(int portNum);
void handleNini(int clientFD);

int main(int argc, char *argv[]){
    if (argc != 2){
        return 1;
    }
    int portNum = atoi(argv[1]);
    printf("%d\n", portNum);

    int FileDescriptor = setup_listen(portNum);

    //int clientFD = acceptConnect(FileDescriptor);

    fd_set all_sockets, prepared_sockets;

    //initialize sets
    FD_ZERO(&all_sockets);
    FD_SET(FileDescriptor, &all_sockets);

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
                    handleNini(iter);
                }
            }
        }
        /*
        //Creating the buffers right here
        char server_buffer[COMMANDINPUTSIZE], client_buffer[COMMANDINPUTSIZE];

        //Clean buffers
        memset(server_buffer, '\0', sizeof(server_buffer));
        memset(client_buffer, '\0', sizeof(client_buffer));

        if(recv(clientFD, client_buffer, sizeof(client_buffer), 0) < 0){
            printf("Ya I couldn't receive\n");
            return -1;
        }
        printf("%s", client_buffer);
        memset(client_buffer, '\0', sizeof(client_buffer));*/
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
    printf("new_socket\n");  //Might want to remove this later on.

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

void handleNini(int clientFD){
    //Creating the buffers right here
    char client_buffer[COMMANDINPUTSIZE];

    //Clean buffers
    memset(client_buffer, '\0', sizeof(client_buffer));

    if(recv(clientFD, client_buffer, sizeof(client_buffer), 0) < 0){
        printf("Ya I couldn't receive\n");
    }
    printf("%s", client_buffer);
    memset(client_buffer, '\0', sizeof(client_buffer));
}