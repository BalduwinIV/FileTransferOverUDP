#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>

#include "packets.h"

#define     BUF_SIZE            1024

#define     TYPE_SYNC           3
#define     TYPE_ACK            1

#define     ONE_SEC             1

#define     ERROR               -1

void send_data(char* local_ip_addr, int local_port, char* dest_ip_addr, int dest_port, char* filename);
void set_ports_and_ip(struct sockaddr_in* client_addr, struct sockaddr_in* server_addr, 
                char* local_ip_addr, int local_port, char* dest_ip_addr, int dest_port);
void send_SYNC_packet(int socket_desc, int local_port, char* filename);
void send_DATA_packet(int socket_desc, char* filename);

void send_data(char* local_ip_addr, int local_port, char* dest_ip_addr, int dest_port, char* filename) {
    // Create socket:
    int socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(socket_desc < 0){
        printf("Unable to create socket\n");
        exit(ERROR);
    }
    printf("Socket created successfully\n");
    
    // Set local and server's port and IP:
    struct sockaddr_in client_addr;
    struct sockaddr_in server_addr;
    set_ports_and_ip(&client_addr, &server_addr, local_ip_addr, local_port, dest_ip_addr, dest_port);
    
    // Bind to the set port and IP:
    if(bind(socket_desc, (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0){
        printf("Couldn't bind to the port\n");
        exit(ERROR);
    }
    printf("Done with binding\n");
    
    // Send connection request to server:
    if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Unable to connect\n");
        exit(ERROR);
    }
    printf("Connected with server successfully\n");

    // Send first (SYNC) packet and get confirmation from the server:
    send_SYNC_packet(socket_desc, local_port, filename);
    
    // Send the remaining (DATA) packets:
    send_DATA_packet(socket_desc, filename);

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

void send_SYNC_packet(int socket_desc, int local_port, char* filename) {

    SYNC_packet_t sync_packet;

    sync_packet.type = TYPE_SYNC;
    sync_packet.type <<= 6;
    sync_packet.sender_port = local_port;
    strCpy(sync_packet.filename, filename);

    char server_message[BUF_SIZE];
    char client_message[BUF_SIZE];

    memcpy(client_message, (char*)&sync_packet, BUF_SIZE);

    ACK_packet_t ack_packet;
    ack_packet.hash = 0;
    ack_packet.state = 0;

    while (true) {
        // Send messege to server:
        if(send(socket_desc, client_message, strlen(client_message), 0) < 0){
            printf("Unable to send message, trying again!\n");
            sleep(ONE_SEC);
            continue;
        }

        // Receive the server's response:
        if(recv(socket_desc, server_message, sizeof(server_message), 0) < 0){
            printf("Error while receiving server's msg\n");
            exit(ERROR);
        }

        // Copy to char array data from server:
        memcpy(&ack_packet, (ACK_packet_t*)server_message, BUF_SIZE);
        
        // Check recieved data:
        if (ack_packet.hash>>30 != TYPE_ACK || ack_packet.state != 1) {
            printf("Something went wrong. Trying to send the first package again\n");
            sleep(ONE_SEC);
        } else {
            printf("The first package was received successfully!\n");
            break;
        }
    }
}

void send_DATA_packet(int socket_desc, char* filename) {
    // Open file stream:
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Error opening file");
        exit(ERROR);
    }

    // Send the DATA to server:
    DATA_packet_t data_packet;
    
    char client_message[BUF_SIZE];

    // Read file's data and send to server:
    data_packet.packet_n = 0;
    while (!feof(file)) {
        fread(data_packet.data, sizeof(char), sizeof(data_packet.data), file);
        data_packet.hash = 0;   // Hash 
        data_packet.CRC = 0;          // and CRC                   
        data_packet.CRC_remainder = 0;         // recuired!!!
        data_packet.packet_n++;

        memcpy(client_message, (char*)&data_packet, BUF_SIZE);
        if(send(socket_desc, client_message, strlen(client_message), 0) < 0){
            printf("Unable to send message\n");
            exit(ERROR);
        }
        
        sleep(ONE_SEC);
    }
    printf("All data was successfully sent!\n");
    // Close file stream:
    fclose(file);
}
