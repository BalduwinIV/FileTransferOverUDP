//./data_sender --ip=10.4.13.216 --port=5000 --dest_ip=10.4.13.239 --dest_port=5000 --filename=vit_normal.ppm send
//./data_sender --ip=127.0.0.1 --port=5007 listen

// ./data_sender --ip=127.0.0.1 --port=5000 --dest_ip=127.0.0.1 --dest_port=5005 --n=10 --filename=vit_small.ppm send
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <sys/select.h>

#include "libcrc-2.0/include/checksum.h"

#include "sender.h"
#include "logger.h"
#include "tools.h"
#include "packets.h"
#include "crc.h"

#define     BUF_SIZE            1024

#ifndef TYPE_SYNC
#define     TYPE_SYNC           3
#endif
#ifndef TYPE_ACK
#define     TYPE_ACK            1
#endif
#ifndef TYPE_DATA
#define     TYPE_DATA           0
#endif

#define     ONE_SEC             1

#define     ERROR               -1
#define     ERROR_FOPEN         200
#define     ERROR_SOCKET        300
#define     ERROR_BIND          301
#define     ERROR_RECEIVE       302

#define     CONFIRM_CODE        0xff
#define     NEGATIVE_CODE       0x00

static char sender_logger[] = "sender.log";
struct sockaddr_in client_addr;
struct sockaddr_in server_addr;
unsigned int length = sizeof(server_addr);

void set_ports_and_ip(struct sockaddr_in* client_addr, struct sockaddr_in* server_addr, 
                char* local_ip_addr, int local_port, char* dest_ip_addr, int dest_port);
void send_SYNC_packet(int socket_desc, int local_port, char* filename, unsigned char *file_hash);
void send_DATA_packet(int socket_desc, char* filename, unsigned char *file_hash, uint32_t CRC, uint8_t packets_buffer_size, SHA256_CTX *sha256);
void send_HASH_packet(int socket_desc, char* filename, unsigned char *file_hash, uint32_t CRC, uint8_t packets_buffer_size, SHA256_CTX *sha256);

void send_data(char* local_ip_addr, int local_port, char* dest_ip_addr, int dest_port, unsigned int CRC, uint8_t packets_buffer_size, char* filename) {
    
    start_logging(sender_logger);

    // Create socket:
    int socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(socket_desc < 0){
        error(sender_logger, "An error occurred while creating socket.");
        stop_logging();
        exit(ERROR_SOCKET);
    }
    info(sender_logger, "Socket has been created successfully.");
    
    // Set local and server's port and IP:
    
    set_ports_and_ip(&client_addr, &server_addr, local_ip_addr, local_port, dest_ip_addr, dest_port);
    
    // Bind to the set port and IP:
    info(sender_logger, "Binding the socket and IP.");
    printf("%d\n", ntohs(client_addr.sin_port));
    if(bind(socket_desc, (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0){
        error(sender_logger, "An error occurred while binding the port and IP.");
        stop_logging();
        exit(ERROR_BIND);

    }
    info(sender_logger, "Binding is done.");
    
    // Send connection request to server:
    info(sender_logger, "Connecting to server.");
    // if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
    //     error(sender_logger, "Unable to connect.\n");
    //     stop_logging();
    //     exit(ERROR);
    // }
    info(sender_logger, "Connected with server successfully.");
    info(sender_logger, "Starting sending...\n");
    fprintf(stderr, "\x1b[38;5;141m\x1b[KStarting sending...\x1b[m\x1b[K\n");

    unsigned char file_hash[SHA256_DIGEST_LENGTH];

    // Send first (SYNC) packet and get confirmation from the server:
    send_SYNC_packet(socket_desc, local_port, filename, file_hash);
    
    // Send the remaining (DATA) packets:
    SHA256_CTX sha256;
    printf("Packets buffer size: %d\n", packets_buffer_size);
    send_DATA_packet(socket_desc, filename, file_hash, CRC, packets_buffer_size, &sha256);
    // send_HASH_packet(socket_desc, filename, file_hash, CRC, packets_buffer_size, &sha256);

    
    info(sender_logger, "All data has been successfully sent!");
    fprintf(stderr, "\x1b[38;5;37m\x1b[KAll data has been successfully sent!\x1b[m\x1b[K\n");
    stop_logging();
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

void send_SYNC_packet(int socket_desc, int local_port, char* filename, unsigned char *file_hash) {
    char log_msg[256];

    SYNC_packet_t sync_packet;

    sync_packet.type = TYPE_SYNC;
    sync_packet.sender_port = 5010;
    memcpy((char *)sync_packet.filename, filename, sizeof(sync_packet.filename));

    // unsigned char server_message[BUF_SIZE];
    // unsigned char client_message[BUF_SIZE];

    // memcpy(client_message, (unsigned char*)&sync_packet, sizeof(SYNC_packet_t));
    
    ACK_packet_t ack_packet;
    // memset(ack_packet.hash, '\0', SHA256_DIGEST_LENGTH);
    ack_packet.state = 0;

    // fd_set read_fds;
    // struct timeval tv;
    fd_set read_fds;
    struct timeval tv;

    while (true) {
        // Send messege to server:
        info(sender_logger, "Sending SYNC packet.");
        
        sync_packet.CRC_remainder = crc_32((unsigned char*)&sync_packet, sizeof(sync_packet)-sizeof(sync_packet.CRC_remainder));
        if(sendto(socket_desc, (unsigned char*)&sync_packet, sizeof(SYNC_packet_t), 0, (struct sockaddr *)&server_addr, length) < 0){
            error(sender_logger, "Unable to send message, trying again!");
            sleep(ONE_SEC);
            continue;
        }
        // printf("SYNC packet CRC: %d\n", sync_packet.CRC_remainder);
        // printf("SYNC packet size: %d\n", sizeof(sync_packet));

        info(sender_logger, "Sending is done.");

        info(sender_logger, "Waiting for response.");

        tv.tv_sec = 0;
        tv.tv_usec = 500*1000;

        FD_ZERO(&read_fds);
        FD_SET(socket_desc, &read_fds);

        if(select(socket_desc+1, &read_fds, NULL, NULL, &tv) > 0){
            if(FD_ISSET(socket_desc, &read_fds)){
                if (recvfrom(socket_desc, &ack_packet, sizeof(ACK_packet_t), 0, (struct sockaddr *)&client_addr, &length) < 0) {
                    error(sender_logger, "Error while receiving server's msg.");
                    exit(ERROR_RECEIVE);
                }
                // sprintf(log_msg, "Packet from %s:%d.", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                // info(sender_logger, log_msg);
                if(crc_32((unsigned char*)&ack_packet, sizeof(ack_packet)-sizeof(ack_packet.CRC_remainder)) != ack_packet.CRC_remainder){
                    warning(sender_logger, "The ACK message CRC does not match. Sending the DATA again.\n");
                    // printf("The ACK message CRC does not match. Sending the DATA again.\n");
                    continue;
                }
                if(ack_packet.state != CONFIRM_CODE) {
                    warning(sender_logger, "The ACK message received contains a negative code. Sending the DATA again.\n");
                }
                if(ack_packet.state == CONFIRM_CODE) {
                    info(sender_logger, "The first package was received successfully!");
                    break;
                }
            }
        } else {
            warning(sender_logger, "TIMEOUT! Sending the SYNC again.\n");
        }
    }
}

void send_DATA_packet(int socket_desc, char* filename, unsigned char *file_hash, unsigned int CRC, uint8_t packets_buffer_size, SHA256_CTX *sha256) {
    char log_msg[256];
    // Open file stream:
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        error(sender_logger, "Error opening file.");
        exit(ERROR_FOPEN);
    }

    struct timespec remaining, request = {0, 1000000};

        /* Send the DATA to server: */
    // Read file's data and send to server:
    DATA_packet_t data_packet;
    ACK_packet_t ack_packet;
    // HASH_packet_t hash_packet;

    // SHA256_Init(&sha256);

    fd_set read_fds;
    struct timeval tv;

    data_packet.packet_n = 0;
    data_packet.type = TYPE_DATA;

    bool sending = true;
    int first_unsent_packet_n = 0;
    int last_packet_n = 0;
    int unconfirmed_packets[packets_buffer_size];
    for(int i = 0; i < packets_buffer_size; i++){
        unconfirmed_packets[i] = -1;
    }
    
    while (true) {
        int unconfirmed_count = 0;
        info(sender_logger, "Starting checking unconfirmed packets list.");
        for(int c = 0; c < packets_buffer_size; c++){
            if(unconfirmed_packets[c] != -1){
                sprintf(log_msg, "DATA packet(%d) was not recieved!", unconfirmed_packets[c]);
                warning(sender_logger, log_msg);
                unconfirmed_count++;
            }
        }
        info(sender_logger, "Ending checking.\n");
    
        if(unconfirmed_count == 0){
            if(!sending){
                info(sender_logger, "All DATA was sent and recieved successful!\n");
                printf("All DATA was sent and recieved successful!\n");
                break;
            }
            info(sender_logger, "All packets from window was recieved!\n");
            
        }
       

        for(int i = 0; i < packets_buffer_size; i++){
            
            if(!last_packet_n && unconfirmed_packets[i] == -1){
                fseek(file, first_unsent_packet_n*sizeof(data_packet.data), SEEK_SET);
                data_packet.packet_n = first_unsent_packet_n;
                data_packet.data_length = fread(&data_packet.data, sizeof(char), sizeof(data_packet.data), file);
                // SHA256_Update(&sha256, (unsigned char*)&data_packet.data, data_packet.data_length);
                if (feof(file)) {
                    info(sender_logger, "Last packet in data detected!");
                    last_packet_n = data_packet.packet_n;
                    data_packet.packet_n |= 0x80000000;
                }
                data_packet.CRC_remainder = crc_32((unsigned char*)&data_packet, sizeof(data_packet)-sizeof(data_packet.CRC_remainder));
                info(sender_logger, "Sending DATA response...");

                sprintf(log_msg, "(DATA[%d]) CRC_remainder = %d", data_packet.packet_n & 0x7fffffff, data_packet.CRC_remainder);
                info(sender_logger, log_msg);
                sprintf(log_msg, "(DATA[%d]) data_length = %d", data_packet.packet_n & 0x7fffffff, data_packet.data_length);
                info(sender_logger, log_msg);
                if(sendto(socket_desc, (unsigned char*)&data_packet, sizeof(DATA_packet_t), 0, (struct sockaddr *)&server_addr, length) < 0){
                    error(sender_logger, "Unable to send DATA message. Trying to send again");
                    sleep(1);
                    continue;
                } else {
                    info(sender_logger, "DATA message has been sent successfully.\n");
                    // nanosleep(&request, &remaining);

                }

                unconfirmed_packets[i] = first_unsent_packet_n;
                first_unsent_packet_n++;
                continue;
            }
            if(unconfirmed_packets[i] != -1){
                fseek(file, unconfirmed_packets[i]*sizeof(data_packet.data), SEEK_SET);
                data_packet.packet_n = unconfirmed_packets[i];
                data_packet.data_length = fread(&data_packet.data, sizeof(char), sizeof(data_packet.data), file);
                if (feof(file)) {
                    info(sender_logger, "Last packet in data!");
                    data_packet.packet_n |= 0x80000000;
                }
                data_packet.CRC_remainder = crc_32((unsigned char*)&data_packet, sizeof(data_packet)-sizeof(data_packet.CRC_remainder));
                info(sender_logger, "Sending DATA response again...");

                sprintf(log_msg, "(DATA[%d]) CRC_remainder = %d", data_packet.packet_n & 0x7fffffff, data_packet.CRC_remainder);
                info(sender_logger, log_msg);
                sprintf(log_msg, "(DATA[%d]) data_length = %d", data_packet.packet_n & 0x7fffffff, data_packet.data_length);
                info(sender_logger, log_msg);

                if(sendto(socket_desc, (unsigned char*)&data_packet, sizeof(DATA_packet_t), 0, (struct sockaddr *)&server_addr, length) < 0){
                    error(sender_logger, "Unable to send DATA message. Trying to send again");
                    sleep(1);
                    continue;
                } else {
                    info(sender_logger, "DATA message has been sent successfully.\n");
                    // nanosleep(&request, &remaining);
                }
            }
        }
        

        // waiting time (100 micro_sec)
        tv.tv_sec = 0;
        tv.tv_usec = 100*1000;

        FD_ZERO(&read_fds);
        FD_SET(socket_desc, &read_fds);

        for(int i = 0; i < packets_buffer_size; i++){
            info(sender_logger, "Waiting for ACK response.");
            if(select(socket_desc+1, &read_fds, NULL, NULL, &tv) > 0){
                if(FD_ISSET(socket_desc, &read_fds)){
                    if (recvfrom(socket_desc, &ack_packet, sizeof(ACK_packet_t), 0, (struct sockaddr *)&client_addr, &length) < 0) {
                        error(sender_logger, "Error while receiving server's msg.");
                        exit(ERROR_RECEIVE);
                    }
                    if(crc_32((unsigned char*)&ack_packet, sizeof(ack_packet)-sizeof(ack_packet.CRC_remainder)) != ack_packet.CRC_remainder){
                        sprintf(log_msg, "The ACK response of data packet(%d) does not match.\n", ack_packet.packet_n);
                        warning(sender_logger, log_msg);
                        // warning(sender_logger, "The ACK message CRC does not match. Sending the DATA again.\n");
                        continue;
                    }
                    if(ack_packet.state != CONFIRM_CODE) {
                        sprintf(log_msg, "The ACK response of packet(%d) contains a negative code.\n", ack_packet.packet_n);
                        warning(sender_logger, log_msg);
                        // warning(sender_logger, "The ACK message received contains a negative code. Sending the DATA again.\n");
                    }
                    if(ack_packet.state == CONFIRM_CODE) {
                        sprintf(log_msg, "The DATA packet(%d) was received successfully.\n", ack_packet.packet_n&0x7fffffff);
                        info(sender_logger, log_msg);

                        // if last DATA packet was confirmd
                        
                        // info(sender_logger, "The DATA packet was received successfully.\n");
                        for(int i = 0; i < packets_buffer_size; i++){
                            if(unconfirmed_packets[i] == (ack_packet.packet_n&0x7fffffff)){
                                // printf("WOOOOORK\n");
                                unconfirmed_packets[i] = -1;
                            }
                        }
                        if((ack_packet.packet_n & 0x80000000)){

                            info(sender_logger, "Server recieved last packet succesfully!\n");
                            printf("Server recieved last packet succesfully!\n");
                            sending = false;
                            break;
                        }
                    }
                }
            } else {
                warning(sender_logger, "TIMEOUT! Sending the DATA again.\n");
                break;
            }
            // for(int i = 0; i < packets_buffer_size; i++){
            //     printf("%d\n", unconfirmed_packets[i]);
            // }
            // printf("\n\n");
        
        }
        
        
        // nanosleep(&request, &remaining);
    }
    sleep(2);
    SHA256_CTX ctx;
    unsigned char buffer[1011];
    unsigned char hash[SHA256_DIGEST_LENGTH];
    int bytes_read;

    fseek(file, 0, SEEK_SET);

    SHA256_Init(&ctx);

    while ((bytes_read = fread(buffer, sizeof(char), 1011, file)) != 0) {
        SHA256_Update(&ctx, buffer, bytes_read);
    }
    HASH_packet_t hash_packet;

    SHA256_Final(hash_packet.hash, &ctx);

    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++){
            sprintf(log_msg, "%2x", hash_packet.hash[i]);
            info(sender_logger, log_msg);
            printf("%2x\n", hash_packet.hash[i]);
    }

    // SHA256_Final(hash, &ctx);
        while(true){


        

        hash_packet.type = TYPE_HASH;
        info(sender_logger, "Sending HASH packet.");
        // SHA256_Final(hash_packet.hash, &sha256);
        hash_packet.CRC_remainder = crc_32((unsigned char*)&hash_packet, sizeof(hash_packet)-sizeof(hash_packet.CRC_remainder));
        if(sendto(socket_desc, (unsigned char*)&hash_packet, sizeof(HASH_packet_t), 0, (struct sockaddr *)&server_addr, length) < 0){
            error(sender_logger, "Unable to send HASH message. Trying to send again");
            sleep(1);
        } else {
            info(sender_logger, "HASH message has been sent successfully.\n");
            // nanosleep(&request, &remaining);
        }
        tv.tv_sec = 0;
        tv.tv_usec = 1000*1000;

        FD_ZERO(&read_fds);
        FD_SET(socket_desc, &read_fds);

        info(sender_logger, "Waiting for (HASH) ACK response.");
        if(select(socket_desc+1, &read_fds, NULL, NULL, &tv) > 0){
            if(FD_ISSET(socket_desc, &read_fds)){
                if (recvfrom(socket_desc, &ack_packet, sizeof(ACK_packet_t), 0, (struct sockaddr *)&client_addr, &length) < 0) {
                    error(sender_logger, "Error while receiving server's msg.");
                    exit(ERROR_RECEIVE);
                }
                if(crc_32((unsigned char*)&ack_packet, sizeof(ack_packet)-sizeof(ack_packet.CRC_remainder)) != ack_packet.CRC_remainder){
                    warning(sender_logger, "The HASH ACK message CRC does not match. Sending the HASH again.\n");
                    continue;
                }
                if(ack_packet.state != CONFIRM_CODE) {
                    warning(sender_logger, "The HASH ACK message received contains a negative code. Sending the ALL DATA again.\n");
                    send_DATA_packet(socket_desc, filename, file_hash, CRC, packets_buffer_size, &sha256);
                }
                if(ack_packet.state == CONFIRM_CODE) {
                    info(sender_logger, "The HASH packet was received successfully.\n");
                    break;
                }
            }
        } else {
            warning(sender_logger, "TIMEOUT! Sending the HASH again.\n");
        }
    }

    // send_HASH_packet(socket_desc, filename, file_hash, CRC, packets_buffer_size, &sha256);
    // char log_msg[256];

        /* Send the DATA to server: */
    // Read file's data and send to server:
    // ACK_packet_t ack_packet;
    // fd_set read_fds;
    // struct timeval tv;
    
    // SHA256_Final(hash_packet.hash, &sha256);
    // hash_packet.CRC_remainder = crc_32((unsigned char*)&hash_packet, sizeof(hash_packet)-sizeof(hash_packet.CRC_remainder));

    
    // while(true){
        // hash_packet.type = TYPE_HASH;
        // info(sender_logger, "Sending HASH packet.");
        // SHA256_Final(hash_packet.hash, &sha256);
        // hash_packet.CRC_remainder = crc_32((unsigned char*)&hash_packet, sizeof(hash_packet)-sizeof(hash_packet.CRC_remainder));
        //  if(sendto(socket_desc, (unsigned char*)&hash_packet, sizeof(HASH_packet_t), 0, (struct sockaddr *)&server_addr, length) < 0){
        //     error(sender_logger, "Unable to send HASH message. Trying to send again");
        //     sleep(1);
        //     // continue;
        // } else {
        //     info(sender_logger, "HASH message has been sent successfully.\n");
        //     // nanosleep(&request, &remaining);
        // }

        // tv.tv_sec = 0;
        // tv.tv_usec = 1000*1000;

        // FD_ZERO(&read_fds);
        // FD_SET(socket_desc, &read_fds);

        // info(sender_logger, "Waiting for (HASH) ACK response.");
        // if(select(socket_desc+1, &read_fds, NULL, NULL, &tv) > 0){
        //     if(FD_ISSET(socket_desc, &read_fds)){
        //         if (recvfrom(socket_desc, &ack_packet, sizeof(ACK_packet_t), 0, (struct sockaddr *)&client_addr, &length) < 0) {
        //             error(sender_logger, "Error while receiving server's msg.");
        //             exit(ERROR_RECEIVE);
        //         }
        //         if(crc_32((unsigned char*)&ack_packet, sizeof(ack_packet)-sizeof(ack_packet.CRC_remainder)) != ack_packet.CRC_remainder){
        //             warning(sender_logger, "The HASH ACK message CRC does not match. Sending the HASH again.\n");
        //             continue;
        //         }
        //         if(ack_packet.state != CONFIRM_CODE) {
        //             warning(sender_logger, "The HASH ACK message received contains a negative code. Sending the ALL DATA again.\n");
        //             send_DATA_packet(socket_desc, filename, file_hash, CRC, packets_buffer_size, &sha256);
        //         }
        //         if(ack_packet.state == CONFIRM_CODE) {
        //             info(sender_logger, "The HASH packet was received successfully.\n");
        //             break;
        //         }
        //     }
        // } else {
        //     warning(sender_logger, "TIMEOUT! Sending the HASH again.\n");
        // }
    // }
    // Close file stream:
    fclose(file);
}

void send_HASH_packet(int socket_desc, char* filename, unsigned char *file_hash, uint32_t CRC, uint8_t packets_buffer_size, SHA256_CTX *sha256) {
    

}