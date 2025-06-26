#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "queue.h"
#include "globals.h"
#include "logger.h"
#include <arpa/inet.h>  
#include <sys/socket.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>

static pthread_t *threads      = NULL;
static int        num_threads  = 0;
static int        shutdown_flag = 0;


// Worker thread function
static void *worker(void *arg)
{
    (void)arg;
    while (1)
    {
        // if (shutdown_flag)
        // {
        //     printf("[worker %ld] popping queue:\n", pthread_self());
        // }
        int client_fd = queue_pop();
        if (client_fd == -1)
            break;               
        
        struct sockaddr_in peer_addr;
        socklen_t peer_len = sizeof(peer_addr);
        char *client_ip = "unknown";
        if (getpeername(client_fd, (struct sockaddr*)&peer_addr, &peer_len) == 0) {
            client_ip = inet_ntoa(peer_addr.sin_addr);
        }
                                // shutdown
        struct timeval tv = {.tv_sec = 1, .tv_usec = 0}; // 1 second timeout
        if (setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO,
                       &tv, sizeof(tv)) < 0)
        {
            perror("setsockopt SO_RCVTIMEO");
        }
        // Read request
        char buf[1024];
        // if (shutdown_flag)
        // {
        //     printf("[worker %ld] reading client_fd:\n", pthread_self());
        // }
        int n = read(client_fd, buf, sizeof(buf) - 1);
        // if (shutdown_flag)
        // {
        //     printf("[worker %ld] read %d bytes. client_fd: %d\n", pthread_self(), n, client_fd);
        // }
        if (n > 0)
        {
            buf[n] = '\0';
            // Parse path
            char method[8], path[256];
            // if (shutdown_flag)
            // {
            //     printf("[worker %ld] sscanf:\n", pthread_self());
            // }
            sscanf(buf, "%s %s", method, path);
            printf("Request: %s\n", path);
            if (strcmp(path, "/") == 0)
                strcpy(path, "/index.html");

            // if (strcmp(method, "GET") != 0 ||
            //     strcmp(path, "/favicon.ico") == 0)
            // {
            //     close(client_fd);
            //     continue;
            // }

            printf("Handled GET %s from worker %lu\n",
                   path, pthread_self());

            // Build full path
            char fullpath[512];
            snprintf(fullpath, sizeof(fullpath), "%s%s", SERVER_ROOT, path);
            // if (shutdown_flag)
            // {
            //     printf("[worker %ld] opening fd:\n", pthread_self());
            // }
            // Try to open
            int fd = open(fullpath, O_RDONLY);
            if (fd < 0)
            {
                // Log error for 404
                logger_log_error(client_ip, method, path, "File not found");
                
                // if (shutdown_flag)
                // {
                //     printf("[worker %ld] writing not found response:\n", pthread_self());
                // }
                const char *not_found =
                    "HTTP/1.0 404 Not Found\r\n"
                    "Content-Type: text/html\r\n\r\n"
                    "<h1>404 Not Found</h1>";
                size_t bytes_sent = write(client_fd, not_found, strlen(not_found));
                
                // Log access with 404 status
                logger_log_access(client_ip, method, path, 404, bytes_sent);
            }
            else
            {
                // if (shutdown_flag)
                // {
                //     printf("[worker %ld] writing response:\n", pthread_self());
                // }
                

                const char *header =
                    "HTTP/1.0 200 OK\r\n"
                    "Content-Type: text/html\r\n\r\n";
                size_t bytes_sent = write(client_fd, header, strlen(header));
                char filebuf[1024];
                int r;
                // if (shutdown_flag)
                // {
                //     printf("[worker %ld] some random read write stuff:\n", pthread_self());
                // }
                while ((r = read(fd, filebuf, sizeof(filebuf))) > 0)
                {
                    // if (shutdown_flag)
                    //     printf("[worker %ld] writing filebuf:\n", pthread_self());
                    int written = write(client_fd, filebuf, r);
                    if (written > 0) {
                        bytes_sent += written;
                    }
                }

                // if (shutdown_flag)
                //     printf("[worker %ld] closing fd:\n", pthread_self());
                close(fd);
                // if (shutdown_flag)
                //     printf("[worker %ld] closed fd:\n", pthread_self());
                
                // Log access with 200 status and bytes sent
                logger_log_access(client_ip, method, path, 200, bytes_sent);
            }
        }
        // if (shutdown_flag)
        // {
        //     printf("[worker %ld] closing client_fd:\n", pthread_self());
        // }
        close(client_fd);
        // if (shutdown_flag)
        // {
        //     printf("[worker %ld] closed client_fd:\n", pthread_self());
        // }
    }
    return NULL;
}

// Start N workers
void threadpool_start(int n) {
    if (n <= 0) n = 1;
    num_threads = n;
    threads = malloc(sizeof(pthread_t) * num_threads);
    if (!threads) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_threads; ++i) {
        if (pthread_create(&threads[i], NULL, worker, NULL) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }
}

// Gracefully stop: wake all workers & join them
void threadpool_stop()
{
    shutdown_flag = 1;
    printf("[pool] stopping, about to shutdown queue\n");
    queue_shutdown();
    printf("[pool] queue_shutdown done, now joining threads\n");
    for (int i = 0; i < num_threads; ++i)
    {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    threads = NULL;
    num_threads = 0;
}
