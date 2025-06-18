#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "queue.h"
#include "globals.h"

#define THREAD_COUNT 4
#define WWW_DIR "./www"

static pthread_t threads[THREAD_COUNT];

// Worker thread function
static void* worker(void* arg) {
    (void)arg;
    while (1) {
        int client_fd = queue_pop();
        if (client_fd == -1) break;  // shutdown

        // Read request
        char buf[1024];
        int n = read(client_fd, buf, sizeof(buf)-1);
        if (n > 0) {
            buf[n] = '\0';
            // Parse path
            char method[8], path[256];
            sscanf(buf, "%s %s", method, path);
            if (strcmp(path, "/") == 0) strcpy(path, "/index.html");

            // Build full path
            char fullpath[512];
            snprintf(fullpath, sizeof(fullpath), "%s%s", WWW_DIR, path);

            // Try to open
            int fd = open(fullpath, O_RDONLY);
            if (fd < 0) {
                const char *not_found =
                    "HTTP/1.0 404 Not Found\r\n"
                    "Content-Type: text/html\r\n\r\n"
                    "<h1>404 Not Found</h1>";
                write(client_fd, not_found, strlen(not_found));
            } else {
                const char *header =
                    "HTTP/1.0 200 OK\r\n"
                    "Content-Type: text/html\r\n\r\n";
                write(client_fd, header, strlen(header));
                char filebuf[1024];
                int r;
                while ((r = read(fd, filebuf, sizeof(filebuf))) > 0)
                    write(client_fd, filebuf, r);
                close(fd);
            }
        }
        close(client_fd);
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
    printf("[pool] stopping, about to shutdown queue\n");
    queue_shutdown();
    printf("[pool] queue_shutdown done, now joining threads\n");
    for (int i = 0; i < THREAD_COUNT; ++i) {
        pthread_join(threads[i], NULL);
    }
}
