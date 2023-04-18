#include "usage.h"

#include <stdio.h>

void print_usage() {
    printf("usage: data_sender [--help] [--ip=LOCAL_IP] [--port=LOCAL_PORT] [--dest_ip=TARGET_IP] [--dest_port=TARGET_PORT] [--file=FILENAME] [listen/send/stop]\n\n");
    printf("options:\n");
    printf("\t--help\t\t\t\t\tShow this help message and exit.\n");
    printf("\t--ip=LOCAL_IP\t\t\t\tSets your ip address to bind it with LOCAL_PORT.\n");
    printf("\t--port=LOCAL_PORT\t\t\tDefines the port that will be open for data transfering.\n");
    printf("\t--dest_ip=TARGET_IP\t\t\tSets the ip adress of a user, who will receive data.\n");
    printf("\t--dest_port=TARGET_PORT\t\t\tDefines the port of a user, who will receive data.\n");
    printf("\t--filename=FILENAME\t\t\t\tDefines the name of file, that yo want to send.\n\n");
    printf("commands:\n");
    printf("\tdata_sender --ip=LOCAL_IP --port=LOCAL_PORT listen\n\t\t\t\t\t\tStart listening.\n");
    printf("\tdata_sender stop\n\t\t\t\t\t\tStop listening.\n");
    printf("\tdata_sender --ip=LOCAL_IP --port=LOCAL_PORT --dest_ip=TARGET_IP --dest_port=TARGET_PORT --filename=FILENAME send\n\t\t\t\t\t\tSend file \"FILENAME\" to TARGET_IP:PORT address.\n");
}
