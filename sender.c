#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>

#include "tools.h"
#include "packets.h"

#define     BUF_SIZE            1024

#define     TYPE_SYNC           3
#define     TYPE_ACK            1

#define     ONE_SEC             1

#define     ERROR               -1

void send_data(char* local_ip_addr, int local_port, char* dest_ip_addr, int dest_port, char* filename);
void set_ports_and_ip(struct sockaddr_in* client_addr, struct sockaddr_in* server_addr, 
                char* local_ip_addr, int local_port, char* dest_ip_addr, int dest_port);
void send_SYNC_packet(int socket_desc, int local_port, char* filename, int *file_hash);
void send_DATA_packet(int socket_desc, char* filename, int file_hash);

void send_data(char* local_ip_addr, int local_port, char* dest_ip_addr, int dest_port, char* filename) {
    // Create socket:
    int socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(socket_desc < 0){
        printf("(sender) Unable to create socket\n");
        exit(ERROR);
    }
    printf("(sender) Socket created successfully\n");
    
    // Set local and server's port and IP:
    struct sockaddr_in client_addr;
    struct sockaddr_in server_addr;
    set_ports_and_ip(&client_addr, &server_addr, local_ip_addr, local_port, dest_ip_addr, dest_port);
    
    // Bind to the set port and IP:
    if(bind(socket_desc, (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0){
        printf("(sender) Couldn't bind to the port\n");
        exit(ERROR);
    }
    printf("(sender) Done with binding\n");
    
    // Send connection request to server:
    if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("(sender) Unable to connect\n");
        exit(ERROR);
    }
    printf("(sender) Connected with server successfully\n");

    int file_hash;

    // Send first (SYNC) packet and get confirmation from the server:
    send_SYNC_packet(socket_desc, local_port, filename, &file_hash);
    
    // Send the remaining (DATA) packets:
    send_DATA_packet(socket_desc, filename, file_hash);

    // Close the socket:
    close(socket_desc);
}

void set_ports_and_ip(struct sockaddr_in* client_addr, struct sockaddr_in* server_addr, 
                char* local_ip_addr, int local_port, char* dest_ip_addr, int dest_port) {
    // Set MY port and IP:
    client_addr->sin_family = AF_INET;
    client_addr->sin_addr.s_addr = inet_addr(local_ip_addr);
    client_addr->sin_port = htons(local_port);

    // Set RECIVER's port and IP:
    server_addr->sin_family = AF_INET;
    server_addr->sin_addr.s_addr = inet_addr(dest_ip_addr);
    server_addr->sin_port = htons(dest_port);
}

void send_SYNC_packet(int socket_desc, int local_port, char* filename, int *file_hash) {
    SYNC_packet_t sync_packet;

    sync_packet.type = TYPE_SYNC;
    sync_packet.sender_port = local_port;
    strcpy(sync_packet.filename, filename);

    unsigned char server_message[BUF_SIZE];
    unsigned char client_message[BUF_SIZE];

    memcpy(client_message, (unsigned char*)&sync_packet, sizeof(SYNC_packet_t));

    ACK_packet_t ack_packet;
    ack_packet.hash = 0;
    ack_packet.state = 0;

    while (true) {
        // Send messege to server:
        if(send(socket_desc, client_message, sizeof(SYNC_packet_t), 0) < 0){
            printf("(sender) Unable to send message, trying again!\n");
            sleep(ONE_SEC);
            continue;
        }

        // Receive the server's response:
        if(recv(socket_desc, server_message, sizeof(server_message), 0) < 0){
            printf("(sender) Error while receiving server's msg\n");
            exit(ERROR);
        }

        // Copy to char array data from server:
        memcpy(&ack_packet, (ACK_packet_t*)server_message, sizeof(ACK_packet_t));
        
        // Check recieved data:
        if (ack_packet.type != TYPE_ACK || ack_packet.state <= 0x40) {
            printf("(sender) Something went wrong. Trying to send the first package again\n");
            sleep(ONE_SEC);
        } else {
            printf("(sender) The first package was received successfully!\n");
            printf("(sender) (ACK) hash = %d\n", ack_packet.hash & 0x3fffffff);

            *file_hash = ack_packet.hash & 0x3fffffff;
            break;
        }
    }
}

void send_DATA_packet(int socket_desc, char* filename, int file_hash) {
    // Open file stream:
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        printf("(sender) Error opening file");
        exit(ERROR);
    }

    // Send the DATA to server:
    DATA_packet_t data_packet;

    ACK_packet_t ack_packet;

    // Read file's data and send to server:
    data_packet.packet_n = 0;
    while (!feof(file)) {
        data_packet.type = TYPE_DATA;
        data_packet.data_length = fread(&data_packet.data, sizeof(char), sizeof(data_packet.data), file);
        if (feof(file)) {
            data_packet.packet_n |= 0x80000000;
        }
        data_packet.hash = file_hash;   // Hash 
        data_packet.CRC = 0;          // and CRC                   
        data_packet.CRC_remainder = 0;         // recuired!!!

        if(send(socket_desc, (unsigned char*)&data_packet, sizeof(DATA_packet_t), 0) < 0){
            printf("(sender) Unable to send message\n");
            exit(ERROR);
        }
        data_packet.packet_n++;

        if(recv(socket_desc, &ack_packet, sizeof(ACK_packet_t), 0) < 0){
            printf("(sender) Error while receiving server's msg\n");
            exit(ERROR);
        }
        printf("(sender) (ACK[%d]) hash = %d\n", ack_packet.packet_n, ack_packet.hash);
        printf("(sender) (ACK[%d]) state = %d\n", ack_packet.packet_n, ack_packet.state);
    }
    printf("(sender) All data has been successfully sent!\n");
    // Close file stream:
    fclose(file);
}
