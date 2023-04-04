#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define     BUF_SIZE            1024

void send_data(char* local_ip_addr, int local_port, char* dest_ip_addr, int dest_port) {
    int socket_desc;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    char server_message[BUF_SIZE];
    char client_message[BUF_SIZE];
    
    // Clean buffers:
    memset(server_message,'\0',sizeof(server_message));
    memset(client_message,'\0',sizeof(client_message));
    
    // Create socket:
    socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    if(socket_desc < 0){
        printf("Unable to create socket\n");
        return -1;
    }
    
    printf("Socket created successfully\n");
    
    // Set MY port and IP:
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = inet_addr(local_ip_addr);
    client_addr.sin_port = htons(local_port);

    // Set RECIVER's port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(dest_ip_addr);
    server_addr.sin_port = htons(dest_port);
    
    // Bind to the set port and IP:
    if(bind(socket_desc, (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0){
        printf("Couldn't bind to the port\n");
        return -1;
    }
    printf("Done with binding\n");
    
    // Send connection request to server:
    if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Unable to connect\n");
        return -1;
    }
    printf("Connected with server successfully\n");
    
    // Get input from the user:
    // printf("Enter message: ");
    scanf("%s", client_message);
    
    // Send the message to server:
    if(send(socket_desc, client_message, strlen(client_message), 0) < 0){
        printf("Unable to send message\n");
        return -1;
    }
    
    // Receive the server's response:
    if(recv(socket_desc, server_message, sizeof(server_message), 0) < 0){
        printf("Error while receiving server's msg\n");
        return -1;
    }
    
    printf("Server's response: %s\n",server_message);
    
    // Close the socket:
    close(socket_desc);
    



    return 0;
}
