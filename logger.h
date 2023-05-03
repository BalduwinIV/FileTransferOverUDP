#ifndef LOGGER_H
#define LOGGER_H

#include <time.h>

/*
 * Start logging in file with given name.
 * @param log_filename  Logger filename.
 * */
void start_logging(char *log_filename);

/*
 * Stop logging in file with given name.
 * @param log_filename  Logger filename.
 * */
void stop_logging_file(char *log_filename);

/*
 * Stop logging in all logger files.
 * */
void stop_logging();

/*
 * Write information message.
 * @param logger_filename   Name of logger to which write message.
 * @param msg               Information message.
 * */
void info(char *logger_filename, char *msg);

/*
 * Write warning message.
 * @param logger_filename   Name of logger to which write message.
 * @param msg               Warning message.
 * */
void warning(char *logger_filename, char *msg);

/*
 * Write error message.
 * @param logger_filename   Name of logger to which write message.
 * @param msg               Error message.
 * */
void error(char *logger_filename, char *msg);

#endif
