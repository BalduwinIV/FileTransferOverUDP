// //./data_sender --ip=10.4.13.216 --port=5000 --dest_ip=10.4.13.239 --dest_port=5000 --filename=vit_normal.ppm send
// //./data_sender --ip=127.0.0.1 --port=5007 listen
// #include <stdlib.h>
// #include <stdio.h>
// #include <string.h>
// #include <string.h>
// #include <sys/socket.h>
// #include <arpa/inet.h>
// #include <unistd.h>
// #include <stdbool.h>
// #include <time.h>
// #include <sys/select.h>

// #include "libcrc-2.0/include/checksum.h"

// #include "logger.h"
// #include "tools.h"
// #include "packets.h"
// #include "crc.h"

// #define     BUF_SIZE            1024

// #ifndef TYPE_SYNC
// #define     TYPE_SYNC           3
// #endif
// #ifndef TYPE_ACK
// #define     TYPE_ACK            1
// #endif
// #ifndef TYPE_DATA
// #define     TYPE_DATA           0
// #endif

// #define     ONE_SEC             1

// #define     ERROR               -1
// #define     ERROR_FOPEN         200
// #define     ERROR_SOCKET        300
// #define     ERROR_BIND          301
// #define     ERROR_RECEIVE       302

// #define     CONFIRM_CODE        0xff
// #define     NEGATIVE_CODE       0x00

// static char sender_logger[] = "sender.log";
// struct sockaddr_in client_addr;
// struct sockaddr_in server_addr;
// unsigned int length = sizeof(server_addr);

// void send_data(char* local_ip_addr, int local_port, char* dest_ip_addr, int dest_port, unsigned int CRC, char* filename);
// void set_ports_and_ip(struct sockaddr_in* client_addr, struct sockaddr_in* server_addr, 
//                 char* local_ip_addr, int local_port, char* dest_ip_addr, int dest_port);
// void send_SYNC_packet(int socket_desc, int local_port, char* filename, unsigned char *file_hash);
// void send_DATA_packet(int socket_desc, char* filename, unsigned char *file_hash, uint32_t CRC);

// void send_data(char* local_ip_addr, int local_port, char* dest_ip_addr, int dest_port, unsigned int CRC, char* filename) {
    
//     start_logging(sender_logger);

//     // Create socket:
//     int socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
//     if(socket_desc < 0){
//         error(sender_logger, "An error occurred while creating socket.");
//         stop_logging();
//         exit(ERROR_SOCKET);
//     }
//     info(sender_logger, "Socket has been created successfully.");
    
//     // Set local and server's port and IP:
    
//     set_ports_and_ip(&client_addr, &server_addr, local_ip_addr, local_port, dest_ip_addr, dest_port);
    
//     // Bind to the set port and IP:
//     info(sender_logger, "Binding the socket and IP.");
//     printf("%d\n", ntohs(client_addr.sin_port));
//     if(bind(socket_desc, (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0){
//         error(sender_logger, "An error occurred while binding the port and IP.");
//         stop_logging();
//         exit(ERROR_BIND);

//     }
//     info(sender_logger, "Binding is done.");
    
//     // Send connection request to server:
//     info(sender_logger, "Connecting to server.");
//     // if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
//     //     error(sender_logger, "Unable to connect.\n");
//     //     stop_logging();
//     //     exit(ERROR);
//     // }
//     info(sender_logger, "Connected with server successfully.");
//     info(sender_logger, "Starting sending...\n");
//     fprintf(stderr, "\x1b[38;5;141m\x1b[KStarting sending...\x1b[m\x1b[K\n");

//     unsigned char file_hash[SHA256_DIGEST_LENGTH];

//     // Send first (SYNC) packet and get confirmation from the server:
//     send_SYNC_packet(socket_desc, local_port, filename, file_hash);
    
//     // Send the remaining (DATA) packets:
//     send_DATA_packet(socket_desc, filename, file_hash, CRC);
    
//     info(sender_logger, "All data has been successfully sent!");
//     fprintf(stderr, "\x1b[38;5;37m\x1b[KAll data has been successfully sent!\x1b[m\x1b[K\n");
//     stop_logging();
//     // Close the socket:
//     close(socket_desc);
// }

// void set_ports_and_ip(struct sockaddr_in* client_addr, struct sockaddr_in* server_addr, 
//                 char* local_ip_addr, int local_port, char* dest_ip_addr, int dest_port) {
//     // Set MY port and IP:
//     client_addr->sin_family = AF_INET;
//     client_addr->sin_addr.s_addr = inet_addr(local_ip_addr);
//     client_addr->sin_port = htons(local_port);

//     // Set RECIVER's port and IP:
//     server_addr->sin_family = AF_INET;
//     server_addr->sin_addr.s_addr = inet_addr(dest_ip_addr);
//     server_addr->sin_port = htons(dest_port);
// }

// void send_SYNC_packet(int socket_desc, int local_port, char* filename, unsigned char *file_hash) {
//     char log_msg[256];

//     SYNC_packet_t sync_packet;

//     sync_packet.type = TYPE_SYNC;
//     sync_packet.sender_port = 5010;
//     memcpy((char *)sync_packet.filename, filename, sizeof(sync_packet.filename));

//     // unsigned char server_message[BUF_SIZE];
//     // unsigned char client_message[BUF_SIZE];

//     // memcpy(client_message, (unsigned char*)&sync_packet, sizeof(SYNC_packet_t));
    
//     ACK_packet_t ack_packet;
//     // memset(ack_packet.hash, '\0', SHA256_DIGEST_LENGTH);
//     ack_packet.state = 0;

//     // fd_set read_fds;
//     // struct timeval tv;
//     fd_set read_fds;
//     struct timeval tv;

//     while (true) {
//         // Send messege to server:
//         info(sender_logger, "Sending SYNC packet.");
        
//         sync_packet.CRC_remainder = crc_32((unsigned char*)&sync_packet, sizeof(sync_packet)-sizeof(sync_packet.CRC_remainder));
//         if(sendto(socket_desc, (unsigned char*)&sync_packet, sizeof(SYNC_packet_t), 0, (struct sockaddr *)&server_addr, length) < 0){
//             error(sender_logger, "Unable to send message, trying again!");
//             sleep(ONE_SEC);
//             continue;
//         }
//         printf("SYNC packet CRC: %d\n", sync_packet.CRC_remainder);
//         printf("SYNC packet size: %d\n", sizeof(sync_packet));

//         info(sender_logger, "Sending is done.");

//         // Receive the server's response:
//         // info(sender_logger, "Waiting for ACK response.");
        
//         // if(recv(socket_desc, &ack_packet, sizeof(ACK_packet_t), 0) < 0){
//         //     error(sender_logger, "Error while receiving server's msg.");
//         //     exit(ERROR_RECEIVE);
//         // }
//         // info(sender_logger, "Response was received.");
//         info(sender_logger, "Waiting for response.");
//         // if (recvfrom(socket_desc, &ack_packet, sizeof(ACK_packet_t), 0, (struct sockaddr *)&client_addr, &length) < 0) {
//         //     error(sender_logger, "Error accepting packet... Interrupting.");
//         //     exit(ERROR_RECEIVE);
//         // }

//         tv.tv_sec = 0;
//         tv.tv_usec = 500*1000;

//         FD_ZERO(&read_fds);
//         FD_SET(socket_desc, &read_fds);

//         if(select(socket_desc+1, &read_fds, NULL, NULL, &tv) > 0){
//             if(FD_ISSET(socket_desc, &read_fds)){
//                 if (recvfrom(socket_desc, &ack_packet, sizeof(ACK_packet_t), 0, (struct sockaddr *)&client_addr, &length) < 0) {
//                     error(sender_logger, "Error while receiving server's msg.");
//                     exit(ERROR_RECEIVE);
//                 }
//                 // sprintf(log_msg, "Packet from %s:%d.", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
//                 // info(sender_logger, log_msg);
//                 if(crc_32((unsigned char*)&ack_packet, sizeof(ack_packet)-sizeof(ack_packet.CRC_remainder)) != ack_packet.CRC_remainder){
//                     warning(sender_logger, "The ACK message CRC does not match. Sending the DATA again.\n");
//                     // printf("The ACK message CRC does not match. Sending the DATA again.\n");
//                     continue;
//                 }
//                 if(ack_packet.state != CONFIRM_CODE) {
//                     warning(sender_logger, "The ACK message received contains a negative code. Sending the DATA again.\n");
//                 }
//                 if(ack_packet.state == CONFIRM_CODE) {
//                     info(sender_logger, "The first package was received successfully!");
//                     // sprintf(log_msg, "(ACK[%d]) hash = %s\n", ack_packet.packet_n & 0x7fffffff, ack_packet.hash);
//                     // info(sender_logger, log_msg);

//                     // memcpy(file_hash, ack_packet.hash, SHA256_DIGEST_LENGTH);
//                     break;
//                 }
//             }
//         } else {
//             warning(sender_logger, "TIMEOUT! Sending the DATA again.\n");
//         }

        

//         // Check recieved data:
//         // if (ack_packet.state <= 0x40) {
//         //     error(sender_logger, "Something went wrong. Trying to send the SYNC package again.\n");
//         //     sleep(ONE_SEC);
//         // } else {
//         //     info(sender_logger, "The first package was received successfully!");
//         //     sprintf(log_msg, "(ACK[%d]) hash = %s\n", ack_packet.packet_n & 0x7fffffff, ack_packet.hash);
//         //     info(sender_logger, log_msg);

//         //     memcpy(file_hash, ack_packet.hash, SHA256_DIGEST_LENGTH);
//         //     break;
//         // }
//     }
// }

// void send_DATA_packet(int socket_desc, char* filename, unsigned char *file_hash, unsigned int CRC) {
//     char log_msg[256];
//     // Open file stream:
//     FILE* file = fopen(filename, "rb");
//     if (file == NULL) {
//         error(sender_logger, "Error opening file.");
//         exit(ERROR_FOPEN);
//     }

//     struct timespec remaining, request = {0, 1000000};

//         /* Send the DATA to server: */
//     // Read file's data and send to server:
//     DATA_packet_t data_packet;
//     ACK_packet_t ack_packet;
//     fd_set read_fds;
//     struct timeval tv;

//     data_packet.packet_n = 0;
//     while (!feof(file)) {
//         data_packet.type = TYPE_DATA;
//         data_packet.data_length = fread(&data_packet.data, sizeof(char), sizeof(data_packet.data), file);
//         if (feof(file)) {
//             data_packet.packet_n |= 0x80000000;
//         }
//         // memcpy(data_packet.hash, file_hash, SHA256_DIGEST_LENGTH);
//         // data_packet.CRC = CRC;          // and CRC                   
//         // data_packet.CRC_remainder = calculate_crc(data_packet.data, data_packet.data_length, data_packet.CRC); // recuired!!!
//         // data_packet.CRC_remainder = crc_32(data_packet.data, data_packet.data_length);
//         data_packet.CRC_remainder = crc_32((unsigned char*)&data_packet, sizeof(data_packet)-sizeof(data_packet.CRC_remainder));

//         while(true){
//             info(sender_logger, "Sending DATA response...");

//             // sprintf(log_msg, "(DATA[%d]) hash = %s", data_packet.packet_n & 0x7fffffff, data_packet.hash);
//             // info(sender_logger, log_msg);
//             // sprintf(log_msg, "(DATA[%d]) CRC = %d", data_packet.packet_n & 0x7fffffff, data_packet.CRC);
//             // info(sender_logger, log_msg);
//             sprintf(log_msg, "(DATA[%d]) CRC_remainder = %d", data_packet.packet_n & 0x7fffffff, data_packet.CRC_remainder);
//             info(sender_logger, log_msg);
//             sprintf(log_msg, "(DATA[%d]) data_length = %d", data_packet.packet_n & 0x7fffffff, data_packet.data_length);
//             info(sender_logger, log_msg);
//             if(sendto(socket_desc, (unsigned char*)&data_packet, sizeof(DATA_packet_t), 0, (struct sockaddr *)&server_addr, length) < 0){
//                 error(sender_logger, "Unable to send DATA message. Trying to send again");
//                 sleep(1);
//                 continue;
//             } else {
//                 info(sender_logger, "DATA message has been sent successfully.");
//             }

//             // waiting time (50 micro_sec)
//             tv.tv_sec = 0;
//             tv.tv_usec = 50*1000;

//             FD_ZERO(&read_fds);
//             FD_SET(socket_desc, &read_fds);

//             info(sender_logger, "Waiting for ACK response.");
//             if(select(socket_desc+1, &read_fds, NULL, NULL, &tv) > 0){
//                 if(FD_ISSET(socket_desc, &read_fds)){
//                     if (recvfrom(socket_desc, &ack_packet, sizeof(ACK_packet_t), 0, (struct sockaddr *)&client_addr, &length) < 0) {
//                         error(sender_logger, "Error while receiving server's msg.");
//                         exit(ERROR_RECEIVE);
//                     }
//                     if(crc_32((unsigned char*)&ack_packet, sizeof(ack_packet)-sizeof(ack_packet.CRC_remainder)) != ack_packet.CRC_remainder){
//                         warning(sender_logger, "The ACK message CRC does not match. Sending the DATA again.\n");
//                         continue;
//                     }
//                     if(ack_packet.state != CONFIRM_CODE) {
//                         warning(sender_logger, "The ACK message received contains a negative code. Sending the DATA again.\n");
//                     }
//                     if(ack_packet.state == CONFIRM_CODE) {
//                         info(sender_logger, "The DATA packet was received successfully.\n");
//                         break;
//                     }
//                 }
//             } else {
//                 warning(sender_logger, "TIMEOUT! Sending the DATA again.\n");
//             }
//         }
        
//         data_packet.packet_n++;
//         // nanosleep(&request, &remaining);
//     }
//     // Close file stream:
//     fclose(file);
// }
