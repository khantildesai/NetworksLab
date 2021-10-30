#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>

#define POLLING 32

typedef struct packet {
    unsigned int total_frag;    //total number of fragments of the file.
    unsigned int frag_no;   //sequence number.
    unsigned int size;  //size of data in range from 0 to 1000.
    char filename[100];
    char filedata[1000];
} packet;

packet* packetMaker(FILE* fptr, int size, char* filename);

void serialize(packet myPacket, char* serialArr);

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
    if (CharArr[4] == '\n'){
        printf("input invalid\n");
	return 1;
    }
    if (CharArr[index + 2] == 'p' && CharArr[index + 1] == 't' && CharArr[index] == 'f'){
        index += 4;

	char *fnameloc = strtok(CharArr + index, "\n");
	
	
	
	
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

    //openning file to get len
    FILE* fPtr;
    fPtr = fopen(fname, "rb");
    fseek(fPtr, 0, SEEK_END);
    int size = ftell(fPtr);
    fseek(fPtr, 0, SEEK_SET);


    //time structs setup for transfer time measurement
    clock_t t;
    
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
    struct timeval tstart;
    int t1 = gettimeofday(&tstart, NULL);
    ByteCount = sendto(FileDescriptor, "ftp", strlen("ftp"), 0, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (ByteCount == -1){
        printf("there was a sendto error!\n");
	return 1;
    }

    memset(CharArr, 0, CharArrSize); //clean buffer
    socklen_t server_addr_size = sizeof(server_addr);

    //recieve the message
    ByteCount = recvfrom(FileDescriptor, CharArr, CharArrSize, 0, (struct sockaddr *) &server_addr, &server_addr_size);
    struct timeval tend;
    int t2 = gettimeofday(&tend, NULL);
    int tdiff = tend.tv_usec - tstart.tv_usec;
    if (ByteCount != -1){
        double rtt = ((double)t)/CLOCKS_PER_SEC;
        printf("round-trip time from the client to the server is %f seconds.\n", rtt);
    }
    if (ByteCount == -1){
        printf("there was a recvfrom error!\n");
        return 1;
    }
    
    if (strcmp(CharArr, "yes") == 0){
        //close(FileDescriptor);
        printf("A file transfer can start\n");
    }
    else{
        //close(FileDescriptor);
        return 0;
    }
    
    //make packets
    packet* packets = packetMaker(fPtr, size, fname);
    int numPackets = packets[0].total_frag;

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 2*tdiff;   //Beej's guide page 50
    if(setsockopt(FileDescriptor, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        printf("setsockopt function has failed\n");
    }

    //send packets
    for (int i = 0; i < numPackets; i++){
	//getting and preparing packet
        packet curr = packets[i];
        char myArr[1200];
	    serialize(curr, myArr);
         
	//sending packet
        ByteCount = sendto(FileDescriptor, myArr, 1200, 0, (struct sockaddr *) &server_addr, sizeof(server_addr));
        
	    if (ByteCount == -1){
            printf("there was a sendto error!\n");
	        return 1;
        }

	    memset(CharArr, 0, CharArrSize); //clearn buffer

        if((ByteCount = recvfrom(FileDescriptor, CharArr, CharArrSize, 0, (struct sockaddr *) &server_addr, &server_addr_size)) == -1){
            printf("Retransmitting last packet due to timeout\n");
            i--;
        }else{
            printf("ACK received\n");
        }


	    // //recieve the message with a time out of wait 0.00006 seconds.
        // ByteCount = recvfrom(FileDescriptor, CharArr, CharArrSize, 0, (struct sockaddr *) &server_addr, &server_addr_size);
        
        // //wait 0.00006 seconds. 
        // //sleep(0.0006);
        
        // if (strcmp(CharArr, "ACK") == 0){
        //     printf("ACK received\n");
        // }else{
        //     printf("Retransmitting last packet.\n");
        //     i -= 1;
        // }
    }
    free(packets);
    
    if (strcmp(CharArr, "yes") == 0){
        close(FileDescriptor);
    //    printf("A file transfer can start\n");
    }
    else{
        close(FileDescriptor);
        return 0;
    }
    fclose(fPtr);
    return 0;
}

packet* packetMaker(FILE* fptr, int size, char* filename){
    FILE* nextPacket = fptr;
    char* buffer = 0;
    buffer = (char*) malloc((size + 1)*sizeof(char));
    //int shitsread = read(fptr, buffer, size + 1);
    fread(buffer, 1, size + 1, fptr);
    int numPackets = size/1000 + (size%1000 != 0);
    
    packet* Packets = calloc(numPackets, sizeof(packet));
     
    int sizeLeftToCopy = size;
    int index = 0;
    for(int packetIter = 0; packetIter < numPackets; packetIter+=1){
        if (sizeLeftToCopy < 1000){
	    //copy only modulo len
	    int lenSet = sizeLeftToCopy%1000;
	    packet temp = {numPackets, packetIter, lenSet};
	    strcpy(temp.filename, filename);
	    temp.filename[99] = '\0';
	    memset(temp.filedata, '\0', 1000);
	    memcpy(temp.filedata, buffer + index, lenSet);
            
            //char test[1001] = {'\0'};
	    //memcpy(test, temp.filedata, 88);


	    Packets[packetIter] = temp;
	    sizeLeftToCopy -= lenSet;
	}
	else{
	    static int iter = 0;
	    iter++;
	    packet temp = {numPackets, packetIter, 1000};
	    strcpy(temp.filename, filename);
	    temp.filename[99] = '\0';
	    memset(temp.filedata, '\0', 1000);
	    memcpy(temp.filedata, buffer + index, 1000);
	    
	    //char test[89] = {'\0'};
	    //memcpy(test, temp.filedata, 88);
	    //printf("whats in this packet: %s\n", test);
	    
	    Packets[packetIter] = temp;
	    //fseek(nextPacket, 88, SEEK_CUR);
	    sizeLeftToCopy -= 1000;
	    index += 1000;
	}
    }

    return Packets;
}

void serialize(packet myPacket, char* serialArr){
    memcpy(serialArr, &(myPacket.total_frag), 4);
    memcpy(serialArr + 4,&(myPacket.frag_no), 4);
    memcpy(serialArr + 8, &(myPacket.size), 4);
    memcpy(serialArr + 12, &(myPacket.filename), 100);
    memcpy(serialArr + 12 + 100, &(myPacket.filedata), 1000);
    //serialArr[1200] = '\0';
}

