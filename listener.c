#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <libgen.h>
#include <pthread.h>
#include <openssl/sha.h>

#include "libcrc-2.0/include/checksum.h"

#include "logger.h"
#include "packets.h"
#include "tools.h"
#include "data_queue.h"
#include "crc.h"

#define     SUCCESS_CODE        0

#define     ERROR_SOCKET        300
#define     ERROR_BIND          301
#define     ERROR_RECEIVE       302
#define     ERROR_CONNECT       303
#define     ERROR_DAEMON        400

#ifndef BUF_SIZE
#define     BUF_SIZE            1024
#endif

#ifndef TYPE_SYNC
#define     TYPE_SYNC           3
#endif
#ifndef TYPE_ACK
#define     TYPE_ACK            1
#endif
#ifndef TYPE_DATA
#define     TYPE_DATA           0
#endif

#define     CONFIRM_CODE        0xff
#define     NEGATIVE_CODE       0x00

static char listener_logger[] = "listener.log";

static void start_daemon();
static SYNC_packet_t * parse_SYNC_packet(unsigned char *client_message, struct sockaddr_in *client_addr);
static void setup_data_file(DATA_file_t *data_owner, SYNC_packet_t *sync_packet);

void start_listener(char *ip_addr, int port) {
    int server_socket;
    struct sockaddr_in server_addr, client_addr;
    unsigned char client_message[BUF_SIZE];
    unsigned int client_message_length = sizeof(client_addr);
    char log_msg[256];

    start_logging(listener_logger);

    /* Clean buffer. */
    memset(client_message, '\0', BUF_SIZE);

    /* Create server socket. */
    server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (server_socket < 0) {
        error(listener_logger, "An error occurred while creating socket.");
        exit(ERROR_SOCKET);
    }
    info(listener_logger, "Socket has been created successfully.");

    sprintf(log_msg, "Setting up server address:\n\tIP = %s\n\tport = %d", ip_addr, port);
    info(listener_logger, log_msg);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip_addr);

    info(listener_logger, "Binding the socket and IP.");
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        error(listener_logger, "An error occurred while binding the port and IP.");
        exit(ERROR_BIND);
    }
    info(listener_logger, "Binding is done.");

    /* Client socket */
    /* client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); */

    info(listener_logger, "Starting listening...");
    fprintf(stderr, "\x1b[38;5;141m\x1b[KStarting listening...\x1b[m\x1b[K\n");

    DATA_file_t *data_owner = safe_malloc(sizeof(DATA_file_t));
    DATA_packet_t data_packet;
    ACK_packet_t ack_packet;
    unsigned short client_port;
    srand(time(NULL));

    _Bool continue_listening = 1;
    while (continue_listening) {
        if (recvfrom(server_socket, client_message, sizeof(client_message), 0, (struct sockaddr *)&client_addr, &client_message_length) < 0) {
            error(listener_logger, "Error accepting packet... Interrupting.");
            exit(ERROR_RECEIVE);
        }
        sprintf(log_msg, "Packet from %s:%d.", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        info(listener_logger, log_msg);

        if (client_message[0] == TYPE_SYNC) {
            info(listener_logger, "Packet type: SYNC.");
            SYNC_packet_t *sync_packet = parse_SYNC_packet(client_message, &client_addr);
            if (crc_32(client_message, sizeof(SYNC_packet_t)-sizeof(sync_packet->CRC_remainder)) == sync_packet->CRC_remainder) {
                ack_packet.type = TYPE_ACK;
                ack_packet.packet_n = 0;
                ack_packet.state = CONFIRM_CODE;
                ack_packet.CRC_remainder = crc_32((unsigned char *)&ack_packet, sizeof(ACK_packet_t)-sizeof(ack_packet.CRC_remainder));

                client_port = ntohs(client_addr.sin_port);

                if (sendto(server_socket, (unsigned char *)&ack_packet, sizeof(ACK_packet_t), 0, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0) {
                    error(listener_logger, "Unable to send ACK message.");
                } else {
                    info(listener_logger, "ACK message has been sent successfully.");
                }

                setup_data_file(data_owner, sync_packet);

                free(sync_packet);
            } else {
                error(listener_logger, "CRC check failed for SYNC packet.");
                ack_packet.type = TYPE_ACK;
                ack_packet.packet_n = 0;
                ack_packet.state = NEGATIVE_CODE;
                ack_packet.CRC_remainder = crc_32((unsigned char *)&ack_packet, sizeof(ACK_packet_t)-sizeof(ack_packet.CRC_remainder));

                if (sendto(server_socket, (unsigned char *)&ack_packet, sizeof(ACK_packet_t), 0, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0) {
                    error(listener_logger, "Unable to send ACK message.");
                } else {
                    info(listener_logger, "ACK message has been sent successfully.");
                }
            }

        } else if (client_message[0] == TYPE_DATA) {
            memcpy(&data_packet, client_message, sizeof(DATA_packet_t));

            printf("Packet #%d\n", data_packet.packet_n & 0x7fffffff);
            info(listener_logger, "Packet type: DATA.");
            sprintf(log_msg, "(DATA[%d]) CRC_remainder = %d", data_packet.packet_n & 0x7fffffff, data_packet.CRC_remainder);
            info(listener_logger, log_msg);
            sprintf(log_msg, "(DATA[%d]) data_length = %d", data_packet.packet_n & 0x7fffffff, data_packet.data_length);
            info(listener_logger, log_msg);

            /* printf("%d\n", crc_32(data_packet.data, data_packet.data_length)); */
            /* if (calculate_crc(data_packet.data, data_packet.data_length, data_packet.CRC) == data_packet.CRC_remainder) { */
            if (crc_32((unsigned char *)&data_packet, sizeof(data_packet)-sizeof(data_packet.CRC_remainder)) == data_packet.CRC_remainder) {
                ack_packet.state = CONFIRM_CODE;
                info(listener_logger, "No problems has been detected while sending a packet.");

                fseek(data_owner->file, (data_packet.packet_n & 0x7fffffff) * sizeof(char) * sizeof(data_packet.data), SEEK_SET);
                fwrite(data_packet.data, sizeof(char), data_packet.data_length, data_owner->file);
                data_owner->packet_n++;

                if (data_packet.packet_n & 0x80000000) {
                    info(listener_logger, "Last packet.");
                    data_owner->max_packet_n = data_packet.packet_n & 0x7fffffff;
                }

                if (data_owner->packet_n >= data_owner->max_packet_n) {
                    fclose(data_owner->file);
                    continue_listening = 0;
                }
            } else {
                ack_packet.state = NEGATIVE_CODE;
                warning(listener_logger, "Packet information has been corrupted. Waiting for the sender response.");
            }

            ack_packet.type = TYPE_ACK;
            ack_packet.packet_n = data_packet.packet_n;
            ack_packet.CRC_remainder = crc_32((unsigned char *)&ack_packet, sizeof(ACK_packet_t)-sizeof(ack_packet.CRC_remainder));
            info(listener_logger, "Sending ACK response...");

            client_addr.sin_family = AF_INET;
            client_addr.sin_addr.s_addr = inet_addr(inet_ntoa(client_addr.sin_addr));
            client_addr.sin_port = htons(client_port);

            if (sendto(server_socket, (unsigned char *)&ack_packet, sizeof(ACK_packet_t), 0, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0) {
                error(listener_logger, "Unable to send ACK message.");
            } else {
                info(listener_logger, "ACK message has been sent successfully.\n");
            }
        } else {
            warning(listener_logger, "Unknown packet type.\n");
        }
    }
    stop_logging();
}

static void start_daemon() {
    pid_t process_id;
    pid_t sid;
    process_id = fork(); /* Starting child process. */
    if (process_id < 0) {
        error(listener_logger, "Error starting daemon...");
        exit(ERROR_DAEMON);
    } else if (process_id > 0) {
        char log_msg[200];
        sprintf(log_msg, "(%d) Daemon started.", process_id);
        info(listener_logger, log_msg);

        FILE *pid_database = fopen("daemons.data", "ab"); /* Save process_id to kill it later. */ 
        if (!pid_database) {
            error(listener_logger, "An error occurred while opening \"daemons.data\" file.");
        }
        fputc(process_id & 0xff, pid_database);
        fputc((process_id >> 8) & 0xff, pid_database);
        fputc((process_id >> 16) & 0xff, pid_database);
        fputc((process_id >> 24) & 0xff, pid_database);
        fclose(pid_database);
        info(listener_logger, "Process number has been written to \"daemons.data\" file.");
    }

    umask(0); /* Reset file mode creation mask. */

    sid = setsid();
    if (sid < 0) {
        exit(ERROR_DAEMON);
    }
    close(STDIN_FILENO);
    /* close(STDERR_FILENO); */
    close(STDOUT_FILENO);

}


void stop_listeners() {
    FILE *pid_database = fopen("daemons.data", "rb");
    if (!pid_database) {
        info(listener_logger, "No processes to stop. Interrupting.");
        exit(SUCCESS_CODE);
    }

    char log_msg[200];
    long int process_id;
    unsigned char byte = fgetc(pid_database);
    while (!feof(pid_database)) {
        process_id = byte + (fgetc(pid_database) << 8) + (fgetc(pid_database) << 16) + (fgetc(pid_database) << 24);
        for (int i = 0; i < 3; i++) {
            if (kill(process_id, SIGTERM) == 0) {
                sprintf(log_msg, "Process %ld has been terminated.", process_id);
                break;
            } else {
                if (i <= 1) {
                    sprintf(log_msg, "Process %ld has not been terminated.\n Trying again.", process_id);
                    error(listener_logger, log_msg);
                    sleep(1);
                } else {
                    sprintf(log_msg, "Unable to terminate process %ld.", process_id);
                    error(listener_logger, log_msg);
                }
            }

        }
        byte = fgetc(pid_database);
    }
    fclose(pid_database);
    remove("daemons.data");
}

static SYNC_packet_t * parse_SYNC_packet(unsigned char *client_message, struct sockaddr_in *client_addr) {
    SYNC_packet_t *sync_packet = (SYNC_packet_t *)safe_malloc(sizeof(SYNC_packet_t));
    memcpy(sync_packet, client_message, sizeof(SYNC_packet_t));
    char log_msg[200];

    sprintf(log_msg, "Filename = %s", sync_packet->filename);
    info(listener_logger, log_msg);
    if (access((char *)sync_packet->filename, F_OK) == 0) {
        warning(listener_logger, "This file does already exists.");
        char new_filename[80];
        uint8_t str_i;
        for (int f_index = 0; f_index < 100; f_index++) {
            strcpy(new_filename, (char *)sync_packet->filename);
            str_i = 0;
            while (sync_packet->filename[str_i] != '.' && sync_packet->filename[str_i] != '\0') {
                str_i++;
            }

            new_filename[str_i] = '(';
            new_filename[str_i+1] = (f_index / 10) + '0';
            new_filename[str_i+2] = (f_index % 10) + '0';
            new_filename[str_i+3] = ')';

            while (sync_packet->filename[str_i] != '\0') {
                new_filename[str_i+4] = sync_packet->filename[str_i];
                str_i++;
            }

            new_filename[str_i+4] = '\0';

            sprintf(log_msg, "Trying to create file \"%s\".", new_filename);
            warning(listener_logger, log_msg);
            
            if (access(new_filename, F_OK) == 0) {
                sprintf(log_msg, "File \"%s\" already exists.", new_filename);
                if (f_index == 99) {
                    sprintf(log_msg, "Could not create file \"%s\".", new_filename);
                }
            } else {
                strcpy((char *)sync_packet->filename, new_filename);
                sprintf(log_msg, "File \"%s\" has been successfully created.", new_filename);
                break;
            }
        }
    }

    client_addr->sin_family = AF_INET;
    client_addr->sin_addr.s_addr = inet_addr(inet_ntoa(client_addr->sin_addr));
    client_addr->sin_port = htons(sync_packet->sender_port);

    return sync_packet;
}

static void setup_data_file(DATA_file_t *data_owner, SYNC_packet_t *sync_packet) {
    data_owner->file = fopen(basename((char *)sync_packet->filename), "wb");
    if (!data_owner->file) {
        error(listener_logger, "Error opening file. Interrupting.");
        exit(ERROR_FOPEN);
    }
    data_owner->packet_n = 0;
    data_owner->max_packet_n = 0 - 1; /* Maximum value of unsigned int. */
}
