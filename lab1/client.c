#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]){
    if (argc != 3){//if less/more than two arg given return 1
        return 1;
    }
    //save command line args
    char serverAddr[100];
    memset(serverAddr, '\0', sizeof(serverAddr));
    strcpy(serverAddr, argv[1]);
    int serverPort = atoi(argv[2]);
    
    //get message to send
    printf("enter ftp message to send in format: ftp <filename> :\n");
    char message[200];
    fgets(message, 200, stdin);

    //check if starts with "ftp "
    char stringIntro[] = "ftp ";
    int equals = 0;
    for (int i = 0; i < 4; i++){
        if (message[i] != stringIntro[i]){
	    equals = 1;
	    printf("%c \n", message[i]);
	    return 1;
	}
    }
    if (equals == 0){
        printf("works \n");
    }

    //check if specified file exists
    //get len of file name
    int nameLen = 0;
    for (int charIter = 0; charIter < 197; charIter++){
        if (message[charIter] == 't' && message[charIter + 1] == 'x' && message[charIter + 2] == 't'){
	    nameLen = charIter;
	}
    }
    printf("%d \n", nameLen);
    char fname[nameLen];
    memset(fname, '\0', sizeof(fname));
    char *messagePtr = message + 4;
    strncpy(fname, messagePtr, nameLen - 1);

    //check if file exists
    FILE * filePtr;
    filePtr = fopen(fname, "r");
    if (filePtr == NULL){
	printf("NOT THERE \n");
        return 1;
    }
    printf("File exists, initiating file transfer!\n");

    //creating socket for client
    int FileDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
    printf("the file descriptor is: %d \n", FileDescriptor);

    unsigned char buf[sizeof(struct in6_addr)];
    long unsigned int ip = inet_pton(AF_INET, serverAddr, buf);

    //creating other data for send to function
    const char FTPChar[] = "ftp";
    unsigned int MessageLen = 1;
    int flags = 0;
    
    //setup structs for sockaddr_in struct
    uint16_t newPortNum = htons(serverPort);
    uint64_t newAddr = htons(ip);
    struct in_addr addrStruct;
    addrStruct.s_addr = newAddr;
    struct sockaddr_in socketAddr;
    socketAddr.sin_family = AF_INET;
    socketAddr.sin_port = (unsigned short) newPortNum;
    socketAddr.sin_addr = addrStruct;

    //pointer to struct
    struct sockaddr_in *addrPtr;
    addrPtr = &socketAddr;
    int bytesSent = sendto(FileDescriptor, FTPChar, MessageLen, flags, (const struct sockaddr *) addrPtr, sizeof(struct sockaddr));
    printf("%d \n", bytesSent);
}
