#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>

#include "logger.h"
#include "packets.h"
#include "usage.h"
#include "tools.h"
#include "listener.h"
#include "sender.h"

#define     DEFAULT_CRC_CODE                0b10000100110000010001110110110111
#define     DEFAULT_PACKETS_BUFFER_SIZE     1

#define     SUCCESS_CODE        0

#define     ERROR_NO_OPERATION  100
#define     ERROR_NO_FILE       101
#define     ERROR_ARGUMENTS     102
#define     ERROR_MALLOC        200
#define     ERROR_SOCKET        300
#define     ERROR_BIND          301
#define     ERROR_RECEIVE       302
#define     ERROR_DAEMON        400

#ifndef     LISTEN
#define     LISTEN              1
#define     STOP                2
#define     SEND_DATA           3
#define     HELP                4
#endif

#ifndef BUF_SIZE
#define     BUF_SIZE            1024
#endif

int main (int argc, char **argv) {
    char *local_ip_addr = (char *)safe_malloc(40*sizeof(char));
    char *dest_ip_addr = (char *)safe_malloc(40*sizeof(char));
    char *filename = (char *)safe_malloc(80*sizeof(char));
    uint32_t CRC = DEFAULT_CRC_CODE; /* CRC-16-IBM */
    uint8_t packets_buffer_size = DEFAULT_PACKETS_BUFFER_SIZE;
    int local_port, dest_port;
    unsigned char operation;
    
    local_ip_addr[0] = '\0';
    local_port = -1;
    dest_ip_addr[0] = '\0';
    dest_port = -1;
    operation = 0;

    parse_args(argc, argv, &local_ip_addr, &local_port, &dest_ip_addr, &dest_port, &CRC, &packets_buffer_size, &filename, &operation);

    if (operation == LISTEN) {
        start_listener(local_ip_addr, local_port);
    } else if (operation == STOP) {
        stop_listeners();
    } else if (operation == SEND_DATA) {
        send_data(local_ip_addr, local_port, dest_ip_addr, dest_port, CRC, filename);
    } else if (operation == HELP) {
        print_usage();
    }

    return SUCCESS_CODE;
}

