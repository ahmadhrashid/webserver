#ifndef QUEUE_H
#define QUEUE_H

void queue_init(int capacity);
void queue_destroy();
void queue_shutdown();
void queue_push(int client_fd);
int queue_pop();

#endif

