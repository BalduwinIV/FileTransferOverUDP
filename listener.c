#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define     SUCCESS_CODE        0

#define     ERROR_SOCKET        300
#define     ERROR_BIND          301
#define     ERROR_RECEIVE       302
#define     ERROR_DAEMON        400

#define     BUF_SIZE            1024

void start_daemon();
void start_listener(char *ip_addr, int port);

void start_daemon() {
    pid_t process_id;
    pid_t sid;
    process_id = fork();
    if (process_id < 0) {
        fprintf(stderr, "Error starting daemon...\n");
        exit(ERROR_DAEMON);
    } else if (process_id > 0) {
        printf("(%d) Daemon started.\n", process_id);
        FILE *pid_database = fopen("daemons.data", "ab");
        fputc(process_id & 0xff, pid_database);
        fputc((process_id >> 8) & 0xff, pid_database);
        fputc((process_id >> 16) & 0xff, pid_database);
        fputc((process_id >> 24) & 0xff, pid_database);
        fclose(pid_database);
    }

    umask(0);

    sid = setsid();
    if (sid < 0) {
        exit(ERROR_DAEMON);
    }
    close(STDIN_FILENO);
    close(STDERR_FILENO);
    close(STDOUT_FILENO);

}

void start_listener(char *ip_addr, int port) {
    int server_socket;
    struct sockaddr_in server_addr, client_addr;
    char client_message[BUF_SIZE];
    unsigned int client_message_length = sizeof(client_addr);

    memset(client_message, '\0', BUF_SIZE);

    server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (server_socket < 0) {
        fprintf(stderr, "An error occurred while creating socket.\n");
        exit(ERROR_SOCKET);
    }
    printf("Socket has been created successfully.");

    printf("Setting up server address:\n\tIP = %s\n\tport = %d\n", ip_addr, port);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip_addr);

    printf("Binding the socket and IP.\n");
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "An error occurred while binding the port and IP.\n");
        exit(ERROR_BIND);
    }
    printf("Binding is done.\n");

    start_daemon();

    FILE *log_file = fopen("listener.log", "w+");

    printf("Starting listening...\n");

    while (1) {
        if (recvfrom(server_socket, client_message, sizeof(client_message), 0, (struct sockaddr *)&client_addr, &client_message_length) < 0) {
            fprintf(stderr, "Something goes wrong... Interrupting.\n");
            exit(ERROR_RECEIVE);
        }

        fprintf(log_file, "Message from %s:%d has been received.\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        fflush(log_file);
    }

    fclose(log_file);
}

void stop_listeners() {
    FILE *pid_database = fopen("daemons.data", "r");
    if (!pid_database) {
        fprintf(stderr, "No processes to stop. Interrupting.\n");
        exit(SUCCESS_CODE);
    }

    long int process_id;
    while (!feof(pid_database)) {
        process_id = fgetc(pid_database) + (fgetc(pid_database) << 8) + (fgetc(pid_database) << 16) + (fgetc(pid_database) << 24);
        for (int i = 0; i < 10; i++) {
            if (kill(process_id, SIGKILL) == 0) {
                printf("Process %ld has been terminated.\n", process_id);
                break;
            } else {
                printf("Process %ld has not been terminated.\n Trying again.\n", process_id);
                sleep(1);
            }

        }
    }
    fclose(pid_database);
    remove("daemons.data");
}
