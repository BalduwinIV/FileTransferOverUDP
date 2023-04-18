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

/* 
 * Inicialize data queue structure.
 * @returns Empty DATA_queue_t structure.
 * */
DATA_queue_t * init_DATA_queue();

/*
 * Add file to queue.
 * @param data_queue    Queue to which file should be added.
 * @param filename      Name of file to add.
 * @param file_hash     That files hash.
 * */
void add_file(DATA_queue_t *data_queue, char *filename, int file_hash);

/*
 * Remove file from queue.
 * @param data_queue    Queue from which file should be removed.
 * @param file_hash     Files hash.
 * */
void remove_file(DATA_queue_t *data_queue, uint32_t file_hash);

/*
 * Find file by its hash.
 * @param data_queue    Queue where file is located (or not).
 * @param file_hash     Files hash.
 * @returns Looked files structure or NULL, if file has not been found.
 * */
DATA_file_t * find_file_by_hash(DATA_queue_t *data_queue, uint32_t file_hash);

/*
 * Free queue.
 * @param data_queue    Queue to free.
 * */
void free_queue(DATA_queue_t *data_queue);

#endif
