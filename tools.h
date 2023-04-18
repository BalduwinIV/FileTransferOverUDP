#ifndef TOOLS_H
#define TOOLS_H

/*
 * Malloc function, that also checks if memory has been successfully allocated.
 * @param capacity  Number of bytes to allocate.
 * @returns Pointer to allocated memory.
 * */
void * safe_malloc(int capacity);

/*
 * Realloc function, that also checks if memory has been successfully allocated.
 * @param ptr       Pointer to a memory to realloc it.
 * @param capacity  Number of bytes to allocate.
 * @returns Pointer to allocated memory.
 * */
void * safe_realloc(void *ptr, int capacity);

/*
 * Parse arguments of program.
 * @param argc          Number of arguments.
 * @param argv          Arguments.
 * @param local_ip_addr "--ip" argument.
 * @param local_port    "--port" argument.
 * @param dest_ip_addr  "--dest_ip" argument.
 * @param dest_port     "--dest_port" argument.
 * @param filename      "--filename" argument.
 * @param operation     Operation argument.
 * */
void parse_args(int argc, char **argv, char **local_ip_addr, int *local_port, char **dest_ip_addr, int *dest_port, char **filename, unsigned char *operation);

#endif
