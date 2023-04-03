#include <openssl/sha.h>

/* (11...) SYN type packet: synchronization before sending data. */
typedef struct {
    unsigned char type; /* 11000000 for this packet type. */
    int sender_port;
    char filename[80];
} SYN_packet_t;

/* (01...) ACK type packet: confirms receipt of data. */
typedef struct {
    int hash; /* first 2 bits defines the packet type (01)
                 other 30 bits are for the hash. */
    int packet_n; /* number of packet the receiver got. */ 
    unsigned char state; /* 11111111 if packet is correct
                            00000000 otherwise. */
} ACK_packet_t;

/* (00...) DATA_type packet: data transfering packet. */
typedef struct {
    int hash; /* first 2 bits defines the packet type (00)
                 other 30 bits are for the hash. */
    int packet_n; /* if first bit is 1 then it is the last packet
                     else if first bit is 0 then server should wait
                     for an another packet. */
    int CRC; /* CRC polynomial. */
    int CRC_remainder;
    char data[896];
} DATA_packet_t;
