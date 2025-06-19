#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>    // for SO_RCVTIMEO
#include <sys/time.h>      // for struct timeval
#include "queue.h"
#include "globals.h"

#define THREAD_COUNT 4
#define WWW_DIR "./www"

int shutdown_flag = 0;  // Global shutdown flag

static pthread_t threads[THREAD_COUNT];

// Worker thread function
static void* worker(void* arg) {
    (void)arg;
    while (1) {
        if (shutdown_flag) {
            printf("[worker %ld] popping queue:\n", pthread_self());
           
        }
        int client_fd = queue_pop();
        if (client_fd == -1) break;  // shutdown
        struct timeval tv = { .tv_sec = 1, .tv_usec = 0 };  // 1 second timeout
        if (setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO,
                    &tv, sizeof(tv)) < 0) {
            perror("setsockopt SO_RCVTIMEO");
        }
        // Read request
        char buf[1024];
        if (shutdown_flag) {
            printf("[worker %ld] reading client_fd:\n", pthread_self());
           
        }
        int n = read(client_fd, buf, sizeof(buf)-1);
        if (shutdown_flag) {
            printf("[worker %ld] read %d bytes. client_fd: %d\n", pthread_self(), n, client_fd);
           
        }
        if (n > 0) {
            buf[n] = '\0';
            // Parse path
            char method[8], path[256];
            if (shutdown_flag) {
            printf("[worker %ld] sscanf:\n", pthread_self());
           
            }
            sscanf(buf, "%s %s", method, path);
            if (strcmp(path, "/") == 0) strcpy(path, "/index.html");

            // Build full path
            char fullpath[512];
            snprintf(fullpath, sizeof(fullpath), "%s%s", WWW_DIR, path);
            if (shutdown_flag) {
            printf("[worker %ld] opening fd:\n", pthread_self());
           
            }
            // Try to open
            int fd = open(fullpath, O_RDONLY);
            if (fd < 0) {
                if (shutdown_flag) {
            printf("[worker %ld] writing not found response:\n", pthread_self());
           
            }
                const char *not_found =
                    "HTTP/1.0 404 Not Found\r\n"
                    "Content-Type: text/html\r\n\r\n"
                    "<h1>404 Not Found</h1>";
                write(client_fd, not_found, strlen(not_found));
            } else {
                if (shutdown_flag) {
            printf("[worker %ld] writing response:\n", pthread_self());
           
            }
                const char *header =
                    "HTTP/1.0 200 OK\r\n"
                    "Content-Type: text/html\r\n\r\n";
                write(client_fd, header, strlen(header));
                char filebuf[1024];
                int r;
                if (shutdown_flag) {
            printf("[worker %ld] some random read write stuff:\n", pthread_self());
           
            }
                while ((r = read(fd, filebuf, sizeof(filebuf))) > 0){
                    if (shutdown_flag) printf("[worker %ld] writing filebuf:\n", pthread_self());
                    write(client_fd, filebuf, r);
                }
                    
                if (shutdown_flag) printf("[worker %ld] closing fd:\n", pthread_self());
                close(fd);
                if (shutdown_flag) printf("[worker %ld] closed fd:\n", pthread_self());
            }
        }
        if (shutdown_flag) {
            printf("[worker %ld] closing client_fd:\n", pthread_self());
           
        }
        close(client_fd);
        if (shutdown_flag) {
            printf("[worker %ld] closed client_fd:\n", pthread_self());
           
        }
    }
    return NULL;
}

// Start THREAD_COUNT workers
void threadpool_start() {
    for (int i = 0; i < THREAD_COUNT; ++i) {
        pthread_create(&threads[i], NULL, worker, NULL);
    }
}

// Gracefully stop: wake all workers & join them
void threadpool_stop() {
    shutdown_flag = 1;  
    printf("[pool] stopping, about to shutdown queue\n");
    queue_shutdown();
    printf("[pool] queue_shutdown done, now joining threads\n");
    for (int i = 0; i < THREAD_COUNT; ++i) {
        pthread_join(threads[i], NULL);
    }
}
