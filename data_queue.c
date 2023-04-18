#include "data_queue.h"
#include <stdlib.h>

#include "tools.h"

DATA_queue_t * init_DATA_queue() {
    DATA_queue_t *data_queue = (DATA_queue_t *)safe_malloc(sizeof(DATA_queue_t));
    data_queue->files_n = 0;
    data_queue->capacity = 1;
    data_queue->files = (DATA_file_t *)safe_malloc(sizeof(DATA_file_t));

    return data_queue;
}

void add_file(DATA_queue_t *data_queue, char *filename, int file_hash) {
    if (data_queue->files_n >= data_queue->capacity) {
        data_queue->files = (DATA_file_t *)safe_realloc(data_queue->files, (data_queue->capacity+1) * sizeof(DATA_file_t));
        data_queue->capacity++;
    }
    data_queue->files[data_queue->files_n].file_hash = file_hash;
    data_queue->files[data_queue->files_n].file = fopen(filename, "wb");
    data_queue->files[data_queue->files_n].packet_n = 0;
    data_queue->files_n++;
}

void remove_file(DATA_queue_t *data_queue, uint32_t file_hash) {
    for (uint32_t file_i = 0; file_i < data_queue->capacity; file_i++) {
        if (data_queue->files[file_i].file_hash == 0) {
            continue;
        } else if (data_queue->files[file_i].file_hash == file_hash) {
            data_queue->files[file_i].file_hash = 0;
            data_queue->files_n--;
            return;
        }
    }
}

DATA_file_t * find_file_by_hash(DATA_queue_t *data_queue, uint32_t file_hash) {
    for (uint32_t file_i = 0; file_i < data_queue->capacity; file_i++) {
        if (data_queue->files[file_i].file_hash == file_hash) {
            return &(data_queue->files[file_i]);
        }
    }
    return NULL;
}

void free_queue(DATA_queue_t *data_queue) {
    free(data_queue->files);
    free(data_queue);
    data_queue = NULL;
}
