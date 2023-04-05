#ifndef DATA_QUEUE_H
#define DATA_QUEUE_H

#include <stdint.h>
#include <stdio.h>

#define     ERROR_FOPEN         200

typedef struct DATA_file_t {
    uint32_t file_hash;
    FILE *file;
    uint32_t packet_n;
} DATA_file_t;

typedef struct {
    uint32_t files_n;
    uint32_t capacity;
    DATA_file_t *files;
} DATA_queue_t;

DATA_queue_t * init_DATA_queue();
void add_file(DATA_queue_t *data_queue, char *filename, int file_hash);
void remove_file(DATA_queue_t *data_queue, uint32_t file_hash);
DATA_file_t * find_file_by_hash(DATA_queue_t *data_queue, uint32_t file_hash);
void free_queue(DATA_queue_t *data_queue);

#endif
