#ifndef PACKETS_H
#define PACKETS_H

#include <stdint.h>
#include <openssl/sha.h>

#define     TYPE_HASH   3
#define     TYPE_SYNC   2
#define     TYPE_ACK    1
#define     TYPE_DATA   0

#pragma pack(push,1)
/* SYNC type packet: synchronization before sending data. */
typedef struct {
    uint8_t type;
    uint32_t sender_port;
    unsigned char filename[80];
    uint32_t CRC_remainder;
} SYNC_packet_t;

/* ACK type packet: confirms receipt of data. */
typedef struct {
    uint8_t type;
    uint32_t packet_n; /* number of packet the receiver got. */ 
    uint8_t state; /* 11111111 if packet is correct 00000000 otherwise. */
    uint32_t CRC_remainder;
} ACK_packet_t;

/* DATA_type packet: data transfering packet. */
typedef struct {
    uint8_t type;
    uint32_t packet_n; /* if first bit is 1 then it is the last packet
                     else if first bit is 0 then server should wait
                     for an another packet. */
    uint32_t data_length;
    unsigned char data[1011];
    uint32_t CRC_remainder;
} DATA_packet_t;

/* HASH_type packet: hash checking packet. */
typedef struct {
    uint8_t type;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    uint32_t CRC_remainder;
} HASH_packet_t;

#pragma pack(pop)
#endif
