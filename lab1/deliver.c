#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>

int main(int argc, char *argv[]){
    if (argc != 3){//if less/more than two arg given return 1
        return 1;
    }
    int serverPort = atoi(argv[2]);
    
    int CharArrSize = 300;
    
    char fname[300];
    
    
    
    memset(fname, 0, 300);
    
    char CharArr[300];
    memset(fname, 0, 300);

    printf("enter ftp command in format: ftp <filename>\n");
    fgets(CharArr, CharArrSize, stdin);

    //checking to make sure user input is proper
    int index = 0;
    if (CharArr[index + 2] == 'p' && CharArr[index + 1] == 't' && CharArr[index] == 'f'){
        index += 4;

	char *fnameloc = strtok(CharArr + index, "\n ");
	
	
	
	
	strncpy(fname, fnameloc, CharArrSize);
    }
    else{
    	printf("input invalid!\n");
	return 1;
    }

    //validity of file
    if(access(fname, F_OK) == -1){
        printf("file doesn't exist!\n");
	return 1;
    }
    //creating socket for client
    int FileDescriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    //setting up required structs
    struct sockaddr_in server_addr;
    memset((char *) &server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(serverPort);
    int convertSucc = inet_aton(argv[1], &server_addr.sin_addr); //converts ip address to integer
    if (convertSucc == 0){
        printf("there was an aton error! \n");
	return 1;
    }

    int ByteCount;

    //send ftp to server
    ByteCount = sendto(FileDescriptor, "ftp", strlen("ftp"), 0, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (ByteCount == -1){
        printf("there was a sendto error!\n");
	return 1;
    }

    memset(CharArr, 0, CharArrSize); //clean buffer
    socklen_t server_addr_size = sizeof(server_addr);

    //recieve the message
    ByteCount = recvfrom(FileDescriptor, CharArr, CharArrSize, 0, (struct sockaddr *) &server_addr, &server_addr_size);
    if (ByteCount == -1){
        printf("there was a recvfrom error!\n");
        return 1;
    }

    if (strcmp(CharArr, "yes") == 0){
        close(FileDescriptor);
        printf("A file transfer can start\n");
    }
    else{
        close(FileDescriptor);
        return 0;
    }
    
    return 0;
}
