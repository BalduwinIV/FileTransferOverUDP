#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define     ERROR_NO_OPERATION  100
#define     ERROR_NO_FILE       101
#define     ERROR_ARGUMENTS     102
#define     ERROR_MALLOC        200
#define     ERROR_REALLOC       201

#define     LISTEN              1
#define     STOP                2
#define     SEND                3
#define     HELP                4

void * safe_malloc(int capacity) {
    void *ptr = malloc(capacity);
    if (!ptr) {
        fprintf(stderr, "Malloc failed!\n");
        exit(ERROR_MALLOC);
    }
    return ptr;
}

void * safe_realloc(void *ptr, int capacity) {
    void *tmp = realloc(ptr, capacity);
    if (!tmp) {
        fprintf(stderr, "Realloc failed. Interrupting.\n");
        exit(ERROR_REALLOC);
    }

    return tmp;
}

void parse_args(int argc, char **argv, char **local_ip_addr, int *local_port, char **dest_ip_addr, int *dest_port, char **filename, unsigned char *operation) {
    int j;
    char *arg;
    for (int i = 1; i < argc; i++) {
        if (strncmp("--ip=", argv[i], 5*sizeof(char)) == 0) {
            arg = argv[i];
            *local_ip_addr = arg+5;
        } else if (strncmp("--port=", argv[i], 7*sizeof(char)) == 0) {
            j = 7;
            *local_port = 0;
            while (argv[i][j] != '\0') {
                *local_port = *local_port * 10 + (argv[i][j] - '0');
                j++;
            }
        } else if (strncmp("--dest_ip=", argv[i], 10*sizeof(char)) == 0) {
            arg = argv[i];
            *dest_ip_addr = arg+10;
        } else if (strncmp("--dest_port=", argv[i], 12*sizeof(char)) == 0) {
            j = 12;
            *dest_port = 0;
            while (argv[i][j] != '\0') {
                *dest_port = *dest_port * 10 + (argv[i][j] - '0');
                j++;
            }
        } else if (strncmp("--filename=", argv[i], 11*sizeof(char)) == 0) {
            arg = argv[i];
            *filename = arg+11;
        } else {
            if (strcmp("listen", argv[i]) == 0) {
                *operation = LISTEN;
            } else if (strcmp("stop", argv[i]) == 0) {
                *operation = STOP;
            } else if (strcmp("send", argv[i]) == 0) {
                if ((*filename)[0] == '\0') {
                    fprintf(stderr, "No file to send has been specified.\n");
                    exit(ERROR_NO_FILE);
                }
                *operation = SEND;
            } else if (strcmp("--help", argv[i]) == 0) {
                *operation = HELP;
            }
        }
    }

    if (*operation == 0) {
        fprintf(stderr, "No operation has been specified.\n");
        exit(ERROR_NO_OPERATION);
    }
    if (*operation == LISTEN && ((*local_ip_addr)[0] == '\0' || *local_port == -1)) {
        fprintf(stderr, "IP address or port has not been specified for this operation.\n");
        exit(ERROR_ARGUMENTS);
    }
    if (*operation == SEND && ((*dest_ip_addr)[0] == '\0' || *dest_port == -1)) {
        fprintf(stderr, "Destination IP address or port has not been specified for this operation.\n");
        exit(ERROR_ARGUMENTS);
    }
}
