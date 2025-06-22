#include "logger.h"
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

static FILE *access_fp, *error_fp;
static pthread_mutex_t log_lock = PTHREAD_MUTEX_INITIALIZER;

void logger_init(const char *access_path, const char *error_path) {
    access_fp = fopen(access_path, "a");
    error_fp  = fopen(error_path,  "a");
    if (!access_fp || !error_fp) {
        perror("logger_init fopen");
        exit(EXIT_FAILURE);
    }
}

static void timestamp(char *buf, size_t len) {
    time_t now = time(NULL);
    strftime(buf, len, "%Y-%m-%d %H:%M:%S", localtime(&now));
}

void logger_log_access(const char *client_ip,
                       const char *method,
                       const char *path,
                       int status,
                       size_t bytes) {
    char ts[20];
    timestamp(ts, sizeof(ts));
    pthread_mutex_lock(&log_lock);
    fprintf(access_fp, "[%s] %s \"%s %s\" %d %zu\n",
            ts, client_ip, method, path, status, bytes);
    fflush(access_fp);
    pthread_mutex_unlock(&log_lock);
}

void logger_log_error(const char *client_ip,
                      const char *method,
                      const char *path,
                      const char *errmsg) {
    char ts[20];
    timestamp(ts, sizeof(ts));
    pthread_mutex_lock(&log_lock);
    fprintf(error_fp, "[%s] %s \"%s %s\" ERROR: %s\n",
            ts, client_ip, method, path, errmsg);
    fflush(error_fp);
    pthread_mutex_unlock(&log_lock);
}

void logger_close() {
    fclose(access_fp);
    fclose(error_fp);
    pthread_mutex_destroy(&log_lock);
}
