#ifndef SENDER_H
#define SENDER_H

void send_data(char* local_ip_addr, int local_port, char* dest_ip_addr, int dest_port, unsigned int CRC, uint8_t packets_buffer_size, char* filename);

#endif
