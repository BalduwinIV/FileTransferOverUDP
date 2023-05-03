#ifndef CRC_H
#define CRC_H

#include <stdint.h>

/*
 * Calculate CRC code from given data.
 * @param data      Data to calculate CRC for.
 * @param data_size Size of data in bytes.
 * @param CRC       CRC polynomial (for CRC-3: 0b00001011).
 * @returns CRC code.
 * */
uint32_t calculate_crc(unsigned char *data, uint32_t data_size, uint32_t CRC);

#endif
