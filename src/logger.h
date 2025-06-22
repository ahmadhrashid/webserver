#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

// Initialize logging (opens files, sets up mutex)
void logger_init(const char *access_path, const char *error_path);

// Log a successful request:
//   client IP, method, path, status code, bytes sent
void logger_log_access(const char *client_ip,
                       const char *method,
                       const char *path,
                       int status,
                       size_t bytes);

// Log an error (e.g., 404, I/O failure):
//   client IP, method, path, error message
void logger_log_error(const char *client_ip,
                      const char *method,
                      const char *path,
                      const char *errmsg);

// Close log files and destroy mutex
void logger_close();

#endif
