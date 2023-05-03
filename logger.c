#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "tools.h"

#define LOGGER_BUFFER_SIZE  8192

typedef struct {
    pthread_t logger_id;
    unsigned char logger_status;
    char **logger_buffer;
    int logger_buffer_size;
} logger_t;

typedef struct {
    int size;
    int capacity;
    char **logger_filenames;
    logger_t *loggers;
} logger_list_t;

static logger_list_t *logger_list = NULL;

static logger_list_t *init_logger_list();
static void add_logger(logger_list_t *logger_list, char *logger_filename);
static void remove_logger(logger_list_t *logger_list, char *logger_filename);
static void free_logger(logger_list_t *logger_list);
static int find_logger(logger_list_t *logger_list, char *logger_filename);

static void *logger_loop(void *log_filename);
static void copy_from_buffer_to_file(FILE *logger, int logger_index);

void start_logging(char *log_filename) {
    if (!logger_list) {
        printf("Starting logger thread...\n");
        logger_list = init_logger_list();
    }
    printf("Adding logger \"%s\"...\n", log_filename);
    FILE *log_file = fopen(log_filename, "w+");
    if (!log_file) {
        fprintf(stderr, "Error occurred while cleaning file %s.\n", log_filename);
    } else {
        fclose(log_file);
    }
    add_logger(logger_list, log_filename);
    printf("Logger \"%s\" has been successfully added.\n", log_filename);
}

void stop_logging_file(char *log_filename) {
    int logger_index = find_logger(logger_list, log_filename);
    if (logger_index == -1) {
        fprintf(stderr, "An error occurred while opening logger file \"%s\".\n", log_filename);
    } else {
        FILE *logger = fopen(log_filename, "a+");
        copy_from_buffer_to_file(logger, logger_index);
        fclose(logger);
    }
    if (logger_list) {
        remove_logger(logger_list, log_filename);
    } else {
        printf("No loggers to remove.\n");
    }
}

void stop_logging() {
    for (int i = 0; i < logger_list->capacity; i++) {
        if (logger_list->logger_filenames[i]) {
            FILE *logger = fopen(logger_list->logger_filenames[i], "a+");
            copy_from_buffer_to_file(logger, i);
            fclose(logger);
        }
    }
    if (logger_list) {
        free_logger(logger_list);
    } else {
        printf("No loggers to remove.\n");
    }
}

void info(char *logger_filename, char *msg) {
    int log_i = find_logger(logger_list, logger_filename);
    time_t rawtime;
    time(&rawtime);

    struct tm *msg_time = localtime(&rawtime);

    char time_buffer[256];
    strcpy(time_buffer, asctime(msg_time));
    time_buffer[strlen(time_buffer)-1] = '\0';

    if (log_i == -1) {
        fprintf(stderr, "[INFO]    %s: %s\n", time_buffer, msg);
    } else {
        sprintf(logger_list->loggers[log_i].logger_buffer[logger_list->loggers[log_i].logger_buffer_size++], "[INFO]    %s: %s\n", time_buffer, msg);
    }
}

void warning(char *logger_filename, char *msg) {
    int log_i = find_logger(logger_list, logger_filename);
    time_t rawtime;
    time(&rawtime);

    struct tm *msg_time = localtime(&rawtime);

    char time_buffer[256];
    strcpy(time_buffer, asctime(msg_time));
    time_buffer[strlen(time_buffer)-1] = '\0';
    
    if (log_i == -1) {
        fprintf(stderr, "[WARNING] %s: %s\n", time_buffer, msg);
    } else {
        sprintf(logger_list->loggers[log_i].logger_buffer[logger_list->loggers[log_i].logger_buffer_size++], "[WARNING] %s: %s\n", time_buffer, msg);
    }
}

void error(char *logger_filename, char *msg) {
    int log_i = find_logger(logger_list, logger_filename);
    time_t rawtime;
    time(&rawtime);

    struct tm *msg_time = localtime(&rawtime);

    char time_buffer[256];
    strcpy(time_buffer, asctime(msg_time));
    time_buffer[strlen(time_buffer)-1] = '\0';

    if (log_i == -1) {
        fprintf(stderr, "[ERROR]   %s: %s\n", time_buffer, msg);
    } else {
        sprintf(logger_list->loggers[log_i].logger_buffer[logger_list->loggers[log_i].logger_buffer_size++], "[ERROR]   %s: %s\n", time_buffer, msg);
    }
}

static void *logger_loop(void *log_filename) {
    FILE *logger_f = fopen((char *)log_filename, "a+");
    int logger_index = find_logger(logger_list, log_filename);
    if (logger_f && logger_index != -1) {
        fprintf(logger_f, "Starting logging...\n");
    } else {
        fprintf(stderr, "An error occurr while starting logging...\n");
    }

    while (logger_list->loggers[logger_index].logger_status) {
        copy_from_buffer_to_file(logger_f, logger_index);
        fclose(logger_f);
        
        sleep(1);
        
        logger_f = fopen((char *)log_filename, "a+");
        if (!logger_f) {
            fprintf(stderr, "An error occurr while opening logger file...");
        }
    }

    if (logger_f) {
        fclose(logger_f);
    }

    return NULL;
}

static void copy_from_buffer_to_file(FILE *logger, int logger_index) {
    int i = 0;
    while (i < logger_list->loggers[logger_index].logger_buffer_size) {
        fprintf(logger, "%s", logger_list->loggers[logger_index].logger_buffer[i]);
        i++;
    }
    logger_list->loggers[logger_index].logger_buffer_size = 0;
}

static logger_list_t *init_logger_list() {
    logger_list_t *return_list = (logger_list_t *)safe_malloc(sizeof(logger_list_t));
    return_list->logger_filenames = NULL;
    return_list->loggers = NULL;
    return_list->size = 0;
    return_list->capacity = 0;

    return return_list;
}

static void add_logger(logger_list_t *logger_list, char *logger_filename) {
    if (logger_list->size == logger_list->capacity) {
        logger_list->logger_filenames = (char **)safe_realloc(logger_list->logger_filenames, sizeof(char *) * (logger_list->capacity+1));
        logger_list->logger_filenames[logger_list->capacity] = (char *)safe_malloc(128 * sizeof(char));

        logger_list->loggers = (logger_t *)safe_realloc(logger_list->loggers, sizeof(logger_t) * (logger_list->capacity+1));

        logger_list->capacity++;
    }

    logger_list->logger_filenames[logger_list->size] = logger_filename;

    logger_list->loggers[logger_list->size].logger_buffer = (char **)safe_malloc(sizeof(char *) * LOGGER_BUFFER_SIZE);
    for (int log_i = 0; log_i < LOGGER_BUFFER_SIZE; log_i++) {
        logger_list->loggers[logger_list->size].logger_buffer[log_i] = (char *)safe_malloc(sizeof(char) * 256);
    }

    logger_list->size++;

    pthread_create(&(logger_list->loggers->logger_id), NULL, logger_loop, logger_filename);

    logger_list->loggers->logger_status = 1;
}

static void remove_logger(logger_list_t *logger_list, char *logger_filename) {
    int log_i;
    for (log_i = 0; log_i < logger_list->capacity; log_i++) {
        if (logger_list->logger_filenames[log_i] && strcmp(logger_filename, logger_list->logger_filenames[log_i]) == 0) {
                break;
        }
    }
    
    if (log_i < logger_list->capacity) {
        fprintf(stderr, "Stopping logger \"%s\"...\n", logger_filename);
        pthread_cancel(logger_list->loggers[log_i].logger_id);
        logger_list->loggers[log_i].logger_status = 0;

        for (int i = 0; i < 10; i++) {
            free(logger_list->loggers[log_i].logger_buffer[i]);
        }
        free(logger_list->loggers[log_i].logger_buffer);
        logger_list->loggers[log_i].logger_buffer = NULL;
        fprintf(stderr, "Logger \"%s\" has been successfully removed...\n", logger_filename);
    }
}

static void free_logger(logger_list_t *logger_list) {
    for (int log_i = 0; log_i < logger_list->capacity; log_i++) {
        if (logger_list->logger_filenames[log_i]) {
            remove_logger(logger_list, logger_list->logger_filenames[log_i]);
        } 
    }

    free(logger_list->logger_filenames);
    free(logger_list->loggers);
    free(logger_list);
    logger_list = NULL;
}

static int find_logger(logger_list_t *logger_list, char *logger_filename) {
    if (!logger_list) {
        return -1;
    }
    for (int log_i = 0; log_i < logger_list->capacity; log_i++) {
        if (logger_list->logger_filenames[log_i] && strcmp(logger_list->logger_filenames[log_i], logger_filename) == 0) {
            return log_i;
        }
    }
    return -1;
}

