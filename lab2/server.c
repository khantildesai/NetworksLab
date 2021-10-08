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
    char filename[100];
    char filedata[88];
} packet;

packet deSerialize(char* serialArr);

char filename[100] = {'\0'};
//memset(filename, '\0', 100);

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


    int currRecv = 0;
    while(1){
        recvfrom(FileDescriptor, buffer, 200, 0, (struct sockaddr *) storageAddressPtr, addressSizePtr);
        
	//checking if it is yes or no to send to the client
        if(strcmp(buffer,"ftp") == 0){
	    printf("recieved message, sending reply!\n");
            sendto(FileDescriptor, "yes", strlen("yes"), 0, (struct sockaddr *) storageAddressPtr, sizeof(storageAddress));
        }
        
	else{
            packet curr = deSerialize(buffer);
	    if (curr.frag_no == currRecv){
	       
	       //printf("gonna send ACK\n");
	       sendto(FileDescriptor, "recieved", strlen("recieved"), 0, (struct sockaddr *) storageAddressPtr, sizeof(storageAddress));
	       
	       int tester = strlen(curr.filedata);
	       //printf("len: %d\n", tester);
	       //printf("gonna save: %d\n", curr.size);
	       char test[81] = {'\0'};
	       memcpy(test, curr.filedata, curr.size);
	       FILE* fp;
	       //printf("toSave: %s\n", test);
	       fp = fopen(curr.filename, "a+");
	       //fputs(test, fp);
	       fwrite(curr.filedata, 1, curr.size, fp);
	       fclose(fp);
	       if (currRecv == curr.total_frag - 1){
	           currRecv = 0;
	       }
	       else {
	           currRecv++;
	       }
	    }
	    //sendto(FileDescriptor, "received", strlen("received"), 0, (struct sockaddr *) storageAddressPtr, sizeof(storageAddress));
        }

    }
}

packet deSerialize(char* serialArr){
    //packet to return
    packet toReturn;
    
    //total_frag
    memcpy(&toReturn.total_frag, serialArr, 4);
    
    //frag_no
    memcpy(&toReturn.frag_no, serialArr + 4, 4);
    
    //size
    memcpy(&toReturn.size, serialArr + 8, 4);
    
    //filename
    memcpy(&toReturn.filename, serialArr + 12, 100);
    
    //filedata
    memcpy(&toReturn.filedata, serialArr + 112, 88);
    //printf("finished deserializing");
    return toReturn;
}

