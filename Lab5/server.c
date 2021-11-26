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

//macros for messages types
#define LOGIN 0
#define LO_ACK 1
#define LO_NACK 2
#define EXIT 3
#define JOIN 4
#define JN_ACK 5
#define JN_NACK 6
#define LEAVE_SESS 7
#define NEW_SESS 8
#define NS_ACK 9
#define MESSAGE 10
#define QUERY 11
#define QU_ACK 12
#define NEW_ACC 13
#define NEW_ACC_ACK 14
#define NEW_ACC_NACK 15

#define COMMANDINPUTSIZE 301 //command line input size

#define MAX_NAME 31 //30 char max username
#define MAX_DATA 301 //300 max data (one line)

//message struct for message format
typedef struct message {
	unsigned int type;
	unsigned int size;
	unsigned char source[MAX_NAME];
	unsigned char data[MAX_DATA];
} message;

//user information struct
typedef struct user{
    char clientID[MAX_NAME];
    char password[MAX_NAME];
    char sessionID[MAX_NAME];
    int online;
    int socket;
} user;

//session information struct
typedef struct session{
    char sessionID[MAX_NAME];
    char admin[MAX_NAME];
} session;

user list_of_users[4] = {{"Joel", "0000", "general", 0, -1}, {"Khantil", "1234", "general", 0, -1}, {"Jane", "1111", "general", 0, -1}, {"Natalie", "Khantil", "general", 0, -1}};

int number_of_sessions = 2;

session list_of_sessions[10] = {{"general", ""}, {"helloworld", ""}};


//keywords for commands
const char login[] = "/login";
const char logout[] = "/logout";
const char joinsession[] = "/joinsession";
const char leavesession[] = "/leavesession";
const char createsession[] = "/createsession";
const char list[] = "/list";
const char quit[] = "/quit";

int acceptConnect(int listeningFD);
int setup_listen(int portNum);
int handleNini(int clientFD);
void messageCreator(int type, char* source, char* data, char *serializedMessage);
void testMessage(message test);
int findIndex(char* username);
int checkSessionExist(char *sessionName);
int checkUserSession(char *sessionNameint, int indexPosition);


//serialize messages function
void serialize(message messagePacket, char* serialArr);

//deserializing message function
message deSerialize(char* serialArr);

fd_set all_sockets, prepared_sockets;

int main(int argc, char *argv[]){
    // const char general[] = "general";
    // printf("len of general is: %d\n", strlen(general));
    // memset(list_of_users[0].sessionID, '\0', 31);
    // memcpy(list_of_users[0].sessionID, general, 8);

    if (argc != 2){
        return 1;
    }
    int portNum = atoi(argv[1]);

    int FileDescriptor = setup_listen(portNum);

    //fd_set all_sockets, prepared_sockets;

    //initialize sets
    FD_ZERO(&all_sockets);
    FD_SET(FileDescriptor, &all_sockets);
    int testflag = 0;
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
                    if (handleNini(iter) == -1){
                        close(iter);
                        FD_CLR(iter, &all_sockets);
                    }
                }
            }
        }
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

int handleNini(int clientFD){
    //Creating the buffers right here
    char client_buffer[400] = {'\0'};
    memset(client_buffer, '\0', 400);
    char server_response[400] = "login success";
    const char check[] = "login";
    const char delim[] = " ";
    char *firstword;

    int numBytesRecv;

    numBytesRecv = recv(clientFD, client_buffer, 400, 0);
    
    // if (numBytesRecv == 0){
    //     printf("connect broken :(\n");
    //     //close(clientFD);
    //     FD_CLR(clientFD, &all_sockets);
    //     return -1;
    // }

    message received_message = deSerialize(client_buffer);

    //testing area
    // received_message.type = NEW_SESS;
    // strcpy(received_message.data, "general");

    printf("received message type: %d\n", received_message.type);
    printf("received message size: %d\n", received_message.size);
    printf("received message source: %s\n",received_message.source);
    printf("received message data: %s\n",received_message.data);

    if(received_message.type == LOGIN){
        printf("it is login command\n");
        int index = -1;
        for(int i = 0; i < 4; i++){
            if(strcmp(list_of_users[i].clientID, received_message.source) == 0){
                printf("username valid\n");
                index = i;
            }
        }
        if(index == -1){
            char reply_message[400] = {'\0'};
            messageCreator(LO_NACK, "server", "wrong username!", reply_message); 
            testMessage(deSerialize(reply_message));
            if(send(clientFD, reply_message, 400, 0) < 0){
                printf("The server failed at responding.\n");
            }
            printf("username not valid\n");
        }else{
            if((strlen(list_of_users[index].password) != strlen(received_message.data)) && strcmp(list_of_users[index].password, received_message.data) != 0){
                char reply_message[400] = {'\0'};
                messageCreator(LO_NACK, "server", "wrong password!", reply_message); 
                testMessage(deSerialize(reply_message));
                if(send(clientFD, reply_message, 400, 0) < 0){
                    printf("The server failed at responding.\n");
                }
                printf("wrong password!\n");
            }
            else{
                list_of_users[index].online = 1;
                list_of_users[index].socket = clientFD;
                printf("online status: %d\n", list_of_users[index].online);
                printf("socket number of user: %d\n", list_of_users[index].socket);
                char reply_message[400] = {'\0'};
                messageCreator(LO_ACK, "server", "", reply_message); 
                testMessage(deSerialize(reply_message));
                // message reply_message;
                // reply_message.type = LO_ACK;
                // reply_message.size = 0;
                // char serializedMessage[400] = {'\0'};
                // serialize(reply_message, serializedMessage);
                if(send(clientFD, reply_message, 400, 0) < 0){
                    printf("The server failed at responding.\n");
                }
                printf("successful login!\n");
            }
        }
    }

    if(received_message.type == JOIN){
        printf("It is the /joinsession command \n");
        int index = findIndex(received_message.source);
        // if((strlen(list_of_users[index].sessionID) + 1 != strlen(received_message.data)) && (strcmp(list_of_users[index].sessionID, received_message.data) != 0) && (checkSessionExist(received_message.data))){
        if(checkSessionExist(received_message.data) && (checkUserSession(received_message.data, index) != 1)){
            char reply_message[400] = {'\0'};
            messageCreator(JN_ACK, "server", received_message.data, reply_message);
            testMessage(deSerialize(reply_message));
            if(send(clientFD, reply_message, 400, 0) < 0){
               printf("The server failed at responding.");
            }
            strcpy(list_of_users[index].sessionID, received_message.data);
            printf("the new joined session ID is: %s\n", list_of_users[index].sessionID);

        }
        else{
            if(checkSessionExist(received_message.data) == 0){
                char reply_message[400] = {'\0'};

                char packet_data[MAX_DATA];
                memset(packet_data, '\0', sizeof(packet_data));
                strcpy(packet_data, received_message.data);
                strcat(packet_data, ", The session ID does not exist!");
                messageCreator(JN_NACK, "server", packet_data, reply_message);
                testMessage(deSerialize(reply_message));
                if(send(clientFD, reply_message, 400, 0) < 0){
                    printf("The server failed at responding.\n");
                }
            }else{
                char reply_message[400] = {'\0'};

                char packet_data[MAX_DATA];
                memset(packet_data, '\0', sizeof(packet_data));
                strcpy(packet_data, received_message.data);
                strcat(packet_data, ", already joined session!");
                messageCreator(JN_NACK, "server", packet_data, reply_message);
                testMessage(deSerialize(reply_message));
                if(send(clientFD, reply_message, 400, 0) < 0){
                    printf("The server failed at responding.\n");
                }
            }   
        }
    }
    if(received_message.type == LEAVE_SESS){
        printf("It is the /leavesession command \n");
        int index = findIndex(received_message.source);
        strcpy(list_of_users[index].sessionID, "null");
        printf("user session is: %s\n", list_of_users[index].sessionID);
    }
    
    if(received_message.type == NEW_SESS){
        printf("It is the /createsession command\n");
        int index = findIndex(received_message.source);
        if(checkSessionExist(received_message.data) == 0){
            number_of_sessions += 1;
            strcpy(list_of_sessions[number_of_sessions-1].sessionID, received_message.data);
            strcpy(list_of_users[index].sessionID, received_message.data);
            printf("session in the list: %s\n", list_of_sessions[number_of_sessions-1].sessionID);
            printf("session in user profile: %s\n", list_of_users[index].sessionID);
            char reply_message[400] = {'\0'};
            messageCreator(NS_ACK, "server", "", reply_message);
            testMessage(deSerialize(reply_message));
            if(send(clientFD, reply_message, 400, 0) < 0){
                printf("The server failed at responding.\n");
            }
        }
        else if(checkSessionExist(received_message.data) == 1){
            printf("this session already exists\n");
        }
    }

    if(received_message.type == MESSAGE){
        printf("server is now dealing with messaging services\n");
        int index = findIndex(received_message.source);
        for(int i = 0; i<4; i++){
            if(i != index){
                printf("it get here\n");
                printf("Name of user: %s, checkUserSession = %d, online? = %d\n", list_of_users[i].clientID, checkUserSession(list_of_users[index].sessionID, i), list_of_users[i].online);
                if(checkUserSession(list_of_users[index].sessionID, i) && (list_of_users[i].online == 1)){
                    printf("this is the second part\n");
                    char reply_message[400];
                    memset(reply_message, '\0', 400);
                    messageCreator(MESSAGE, "server", received_message.data, reply_message);
                    testMessage(deSerialize(reply_message));
                    printf("testing raw string: %d %d\n", *reply_message, *(reply_message + sizeof(unsigned int)));
                    if(send(list_of_users[i].socket, reply_message, 400, 0) < 0){
                        printf("The server failed at responding.\n");
                    }
                }
            }
        }
    }

    if(received_message.type == QUERY){
        printf("server is now dealing with /list command\n");
        char reply_message[400] = {'\0'};
        char packet_data[MAX_DATA];
        memset(packet_data, '\0', sizeof(packet_data));
        strcpy(packet_data, "Clients: ");
        for(int i = 0; i<4; i++){
            if(list_of_users[i].online){
                strcat(packet_data, list_of_users[i].clientID);
                strcat(packet_data, " ");
            }
        }
        strcat(packet_data, "Sessions: ");
        for(int i = 0; i<number_of_sessions; i++){
            strcat(packet_data, list_of_sessions[i].sessionID);
            strcat(packet_data, " ");
        }
        
        messageCreator(QU_ACK, "server", packet_data, reply_message);
        testMessage(deSerialize(reply_message));
        if(send(clientFD, reply_message, 400, 0) < 0){
            printf("The server failed at responding.\n");
        }
    }

    if(received_message.type == EXIT){
        int index = findIndex(received_message.source);
        list_of_users[index].online = 0;
        list_of_users[index].socket = -1;
        printf("man's been logged out \n");
        //close(clientFD);
        FD_CLR(clientFD, &all_sockets);
    }
    //firstword = strtok(client_buffer, delim);

    //if(strcmp(firstword, check) == 0){
    //    if(send(clientFD, server_response, strlen(server_response), 0) < 0){
    //        printf("The server failed at responding.");
    //    }
    //}
    memset(client_buffer, '\0', sizeof(client_buffer));
    memset(server_response, '\0', sizeof(server_response));
    memset(received_message.source, '\0', sizeof(received_message.source));
    memset(received_message.data, '\0', sizeof(received_message.data));
}

void serialize(message messagePacket, char* serialArr){
	memset(serialArr, '\0', 400);
	memcpy(serialArr, &messagePacket.type, sizeof(unsigned int)); //type
	memcpy(serialArr + sizeof(unsigned int), &(messagePacket.size), sizeof(unsigned int)); //size
	memcpy(serialArr + 2*sizeof(unsigned int), &(messagePacket.source), MAX_NAME*sizeof(unsigned int)); //source
	memcpy(serialArr + 2*sizeof(unsigned int) + MAX_NAME*sizeof(unsigned int), &(messagePacket.data), messagePacket.size); //data
}

message deSerialize(char* serialArr){
	//packet to return
	message toReturn;

	//type
	memcpy(&toReturn.type, serialArr, sizeof(unsigned int));

	//size
	memcpy(&toReturn.size, serialArr + sizeof(unsigned int), sizeof(unsigned int));

	//source
	memcpy(&toReturn.source, serialArr + 2*sizeof(unsigned int), MAX_NAME*sizeof(unsigned char));

	//data
	memcpy(&toReturn.data, serialArr + 2*sizeof(unsigned int) + MAX_NAME*sizeof(unsigned char), toReturn.size);

    return toReturn;
}

void messageCreator(int type, char* source, char* data, char *serializedMessage){
    message reply;
    reply.type = type;
    reply.size = strlen(data);
    strcpy(reply.source, source);
    memcpy(reply.data, data, reply.size);
    serialize(reply, serializedMessage);
}

void testMessage(message test){
    printf("test.type = %d\n", test.type);
    printf("test.size = %d\n", test.size);
    printf("test.source = %s\n", test.source);
    printf("test.data = %s\n", test.data);
}

int findIndex(char* username){
    int result = -1;
    for(int i = 0; i<4; i++){
        if(strcmp(username, list_of_users[i].clientID) == 0){
            result = i;
        }
    }
    return result;
}

int checkSessionExist(char *sessionName){
    int result = 0;
    for(int i = 0; i < number_of_sessions; i++){
        char *ret;
        ret = strstr(sessionName, list_of_sessions[i].sessionID);
        if(ret != NULL && (strlen(list_of_sessions[i].sessionID)+1 == strlen(sessionName))){
            result = 1;
        }else if(ret != NULL && (strlen(list_of_sessions[i].sessionID) == strlen(sessionName))){
            result = 1;
        }
    }
    // printf("checkSessionExist function is being called:\n");
    // printf("the string received is: %s and the length is %d\n", sessionName, strlen(sessionName));
    // printf("the string in database is: %s and the length is %d\n", list_of_sessions[1].sessionID, strlen(list_of_sessions[1].sessionID));
    return result;
}

int checkUserSession(char *sessionName, int indexPosition){
    int result = 0;
    char *ret;
    ret = strstr(sessionName, list_of_users[indexPosition].sessionID);
    if(ret != NULL && (strlen(list_of_users[indexPosition].sessionID)+1 == strlen(sessionName))){
        result = 1;
    }
    else if(ret != NULL && (strlen(list_of_users[indexPosition].sessionID) == strlen(sessionName))){
        result = 1;
    }
    return result;
}