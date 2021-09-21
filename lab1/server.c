#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]){
    if (argc != 2){ //if less/more than 1 arg given; return
        return 1;
    }
    int portNum = atoi(argv[1]);
    printf("the port number is: %d \n", portNum);

    int FileDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
    printf("the file descriptor is: %d \n", FileDescriptor);

    //set up structs
    unsigned char buf[sizeof(struct in6_addr)];
    long unsigned int ip = inet_pton(AF_INET, "128:100:13:180", buf);

    //making sockaddr_in struct
    uint16_t newPortNum = htons(portNum);
    uint64_t newAddr = htons(ip);
    struct in_addr addrStruct;
    addrStruct.s_addr = newAddr;
    struct sockaddr_in socketAddr;
    socketAddr.sin_family = AF_INET;
    socketAddr.sin_port = (unsigned short) portNum;
    socketAddr.sin_addr = addrStruct;

    //pointer to struct
    struct sockaddr_in *addrPtr;
    addrPtr = &socketAddr;
    int bindSucc = bind(FileDescriptor, (const struct sockaddr *)addrPtr, sizeof(struct sockaddr));
    printf("%d \n", bindSucc);
}
