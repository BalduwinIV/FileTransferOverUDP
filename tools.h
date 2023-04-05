#ifndef TOOLS_H
#define TOOLS_H

void * safe_malloc(int capacity);
void parse_args(int argc, char **argv, char **local_ip_addr, int *local_port, char **dest_ip_addr, int *dest_port, char **filename, unsigned char *operation);
void * safe_realloc(void *ptr, int capacity);

#endif
