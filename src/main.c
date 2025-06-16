#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#define PORT 8080

int main() {
    int server_fd, client_fd;
    char buffer[1024];
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket failed");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 5);

    printf("Listening on port %d...\n", PORT);

    client_fd = accept(server_fd, (struct sockaddr *)&addr, &addrlen);
    read(client_fd, buffer, sizeof(buffer));
    printf("Request:\n%s\n", buffer);

    const char *response =
        "HTTP/1.0 200 OK\r\n"
        "Content-Type: text/html\r\n\r\n"
        "<h1>Hello from your web server!</h1>";

    write(client_fd, response, strlen(response));
    close(client_fd);
    close(server_fd);

    return 0;
}

