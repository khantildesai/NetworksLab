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
    if (argc != 3){//if less/more than two arg given return 1
        return 1;
    }
    //save command line args
    //char serverAddr[100];
    //memset(serverAddr, '\0', sizeof(serverAddr));
    //strcpy((serverAddr + 4), argv[1]);
    int serverPort = atoi(argv[2]);
    
    //get message to send
    //printf("enter ftp message to send in format: ftp <filename> :\n");
    //char message[200];
    //fgets(message, 200, stdin);

    //check if starts with "ftp "
    //char stringIntro[] = "ftp ";
    //int equals = 0;
    //for (int i = 0; i < 4; i++){
    //    if (message[i] != stringIntro[i]){
    //	    equals = 1;
    //	    printf("%c \n", message[i]);
    //	    return 1;
    //	}
    //}
    //if (equals == 0){
    //    printf("works \n");
    //}

    //check if specified file exists
    //get len of file name
    //int nameLen = 0;
    //for (int charIter = 0; charIter < 197; charIter++){
    //    if (message[charIter] == 't' && message[charIter + 1] == 'x' && message[charIter + 2] == 't'){
    //	    nameLen = charIter;
    //	}
    //}
    /*printf("%d \n", nameLen);
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
    */
    //creating socket for client
    int FileDescriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (FileDescriptor == -1){
        printf("socket error!\n");
	return 1;
    }

    //setting up required structs
    struct sockaddr_in server_addr;
    memset((char *) &server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(serverPort);
    if (inet_aton(argv[1], &server_addr.sin_addr) == 0){
        printf("inet_aton error! \n");
	return 1;
    }

    //get user input
    const int BUFF_SIZE = 300;
    char buffer[300] = {0};
    char filename[300] = {0};

    printf("enter ftp command in format: ftp <filename>\n");
    fgets(buffer, BUFF_SIZE, stdin);

    //checking to make sure user input is proper
    int cursor = 0;
    if (buffer[cursor] == 'f' && buffer[cursor + 1] == 't' && buffer[cursor + 2] == 'p'){
        cursor += 4;

	char *fnameloc = strtok(buffer + cursor, "\r\t\n ");
	strncpy(filename, fnameloc, BUFF_SIZE);
    } else {
    	printf("input invalid!");
	return 1;
    }

    //validity of file
    if(access(filename, F_OK) == -1){
        printf("file doesn't exist!");
	return 1;
    }

    int numbytes;
    //send the message
    numbytes = sendto(FileDescriptor, "ftp", strlen("ftp"), 0, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (numbytes == -1){
        printf("sendto error!\n");
	return 1;
    }
    printf("sent!\n");

    memset(buffer, 0, BUFF_SIZE); //clean buffer
    socklen_t server_addr_size = sizeof(server_addr);

    numbytes = recvfrom(FileDescriptor, buffer, BUFF_SIZE, 0, (struct sockaddr *) &server_addr, &server_addr_size);
    if (numbytes == -1){
        printf("recvfrom error!\n");
        return 1;
    }

    if (strcmp(buffer, "yes") == 0){
        printf("A file transfer can start\n");
    }
    close(FileDescriptor);

    return 0;
    /*
    unsigned char buf[sizeof(struct in6_addr)];
    long unsigned int ip = inet_pton(AF_INET, serverAddr, buf);

    //creating other data for send to function
    const char FTPChar[] = "ftp";
    unsigned int MessageLen = 3;
    int flags = 0;
    
    //setup structs for sockaddr_in struct
    uint16_t newPortNum = htons(serverPort);
    uint64_t newAddr = htons(ip);
    struct in_addr addrStruct;
    addrStruct.s_addr = newAddr;
    struct sockaddr_in socketAddr;
    //memset((char *) &socketAddr, 0, sizeof(socketAddr));
    socketAddr.sin_family = AF_INET;
    socketAddr.sin_port = (unsigned short) newPortNum;
    socketAddr.sin_addr = addrStruct;

    //pointer to struct
    struct sockaddr_in *addrPtr;
    addrPtr = &socketAddr;
    int bytesSent = sendto(FileDescriptor, FTPChar, MessageLen, flags, (const struct sockaddr *) &addrPtr, sizeof(struct sockaddr));
    printf("%d \n", bytesSent);
    printf("%d \n", errno);*/
}
