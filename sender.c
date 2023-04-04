#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "packets.h"

#define     BUF_SIZE            1024
#define     DATA_SIZE           896

#define     TYPE_SYNC           3
#define     TYPE_ACK            1

void send_data(char* local_ip_addr, int local_port, char* dest_ip_addr, int dest_port, char* filename) {
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
        exit(-1);
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
        exit(-1);
    }
    printf("Done with binding\n");
    
    // Send connection request to server:
    if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Unable to connect\n");
        exit(-1);
    }
    printf("Connected with server successfully\n");

    // Create first packet:
    SYNC_packet_t sync_packet;
    sync_packet.type = TYPE_SYNC;
    sync_packet.type <<= 6;
    sync_packet.sender_port = local_port;
    strCpy(sync_packet.filename, filename);

    memcpy(client_message, (char*)&sync_packet, BUF_SIZE);

    ACK_packet_t ack_packet;
    ack_packet.hash = 0;
    ack_packet.state = 0;

    while (ack_packet.hash>>30 != TYPE_ACK || ack_packet.state == 0) {
        if(send(socket_desc, client_message, strlen(client_message), 0) < 0){
            printf("Unable to send message\n");
            exit(-1);
        }

        // Receive the server's response:
        if(recv(socket_desc, server_message, sizeof(server_message), 0) < 0){
            printf("Error while receiving server's msg\n");
            exit(-1);
        }

        memcpy(&ack_packet, (ACK_packet_t*)server_message, BUF_SIZE);
    }
    
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Error opening file");
        exit(-1);
    }

    // Send the message to server:
    DATA_packet_t data_packet;
    data_packet.packet_n = 0;
    while (!feof(file)) {
        fread(data_packet.data, sizeof(char), DATA_SIZE, file);
        data_packet.hash = 0;
        data_packet.CRC = 0;
        data_packet.CRC_remainder = 0;
        data_packet.packet_n++;

        memcpy(client_message, (char*)&data_packet, BUF_SIZE);
        if(send(socket_desc, client_message, strlen(client_message), 0) < 0){
            printf("Unable to send message\n");
            exit(-1);
        }
    }
    fclose(file);
    
    // Close the socket:
    close(socket_desc);
    



    return 0;
}
