#ifndef PACKETS_H
#define PACKETS_H

#include <stdint.h>

#define     TYPE_SYNC   3
#define     TYPE_ACK    1
#define     TYPE_DATA   0

#pragma pack(push,1)
/* (11...) SYNC type packet: synchronization before sending data. */
typedef struct {
    uint8_t type; /* 11000000 for this packet type. */
    uint32_t sender_port;
    char filename[80];
} SYNC_packet_t;

/* (01...) ACK type packet: confirms receipt of data. */
typedef struct {
    uint8_t type;
    uint32_t hash; /* first 2 bits defines the packet type (01)
                      other 30 bits are for the hash. */
    uint32_t packet_n; /* number of packet the receiver got. */ 
    uint8_t state; /* 11111111 if packet is correct
                            00000000 otherwise. */
} ACK_packet_t;

/* (00...) DATA_type packet: data transfering packet. */
typedef struct {
    uint8_t type;
    uint32_t hash; /* first 2 bits defines the packet type (00)
                 other 30 bits are for the hash. */
    uint32_t packet_n; /* if first bit is 1 then it is the last packet
                     else if first bit is 0 then server should wait
                     for an another packet. */
    uint32_t data_length;
    uint32_t CRC; /* CRC polynomial. */
    uint32_t CRC_remainder;
    unsigned char data[1003];
} DATA_packet_t;

#pragma pack(pop)
#endif
