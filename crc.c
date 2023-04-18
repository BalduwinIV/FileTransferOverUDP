#include "crc.h"

uint32_t calculate_crc(unsigned char *data, uint32_t data_size, uint32_t CRC) {
    uint32_t CRC_top_bit = 0x80000000;
    while (!(CRC_top_bit & CRC)) CRC_top_bit >>= 1;

    uint8_t CRC_remainder = 0;
    uint8_t byte_mask;
    for (uint16_t i = 0; i < data_size; i++) {
        byte_mask = 0b10000000;
        do {
            CRC_remainder <<= 1;
            CRC_remainder += data[i] & byte_mask ? 1 : 0;
            byte_mask >>= 1;
            if (CRC_remainder & CRC_top_bit) {
                CRC_remainder ^= CRC;
            }
        } while (byte_mask);
    }
    return CRC_remainder;
}
