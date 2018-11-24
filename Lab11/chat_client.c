#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#ifndef PORT
  #define PORT 30000
#endif
#define BUF_SIZE 128

int main(void) {
    // Create the socket FD.
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("client: socket");
        exit(1);
    }

    // Set the IP and port of the server to connect to.
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &server.sin_addr) < 1) {
        perror("client: inet_pton");
        close(sock_fd);
        exit(1);
    }

    // Connect to the server.
    if (connect(sock_fd, (struct sockaddr *)&server, sizeof(server)) == -1) {
        perror("client: connect");
        close(sock_fd);
        exit(1);
    }

    // Read input from the user, send it to the server, and then accept the
    // echo that returns. Exit when stdin is closed.
    // Write the user name to server on connected.
    char buf[BUF_SIZE + 1];
    int name_read = read(STDIN_FILENO, buf, BUF_SIZE);
    buf[name_read] = '\0';
    int name_write = write(sock_fd, buf, BUF_SIZE);
    if (name_write != BUF_SIZE) {
        perror("client: write name");
        close(sock_fd);
        exit(1);
    }
    int max_fd = STDIN_FILENO;
    if (sock_fd > max_fd) {
        max_fd = sock_fd;
    }
    fd_set all_fds;
    FD_ZERO(&all_fds);
    FD_SET(sock_fd, &all_fds);
    FD_SET(STDIN_FILENO, &all_fds);

    while (1) {
        fd_set fds = all_fds;
        int nready = select(max_fd + 1, &fds, NULL, NULL, NULL);
        if (nready == -1) {
            perror("Client: select");
            exit(1);
        }

        if (FD_ISSET(sock_fd, &fds)) {
            int num_read = read(sock_fd, buf, BUF_SIZE);
            buf[num_read] = '\0';
            printf("Received from server: %s", buf);
        } else if (FD_ISSET(STDIN_FILENO, &fds)) {
            int num_read = read(STDIN_FILENO, buf, BUF_SIZE);
            if (num_read == 0) {
                break;
            }
            int nwrite = write(sock_fd, buf, BUF_SIZE);
            if (nwrite != BUF_SIZE) {
                perror("Write to server");
                exit(1);
            }
        }
        // Reset buf to all \0;
        for (int i = 0; i < BUF_SIZE + 1; i++) {
            buf[i] = '\0';
        }
    }

    close(sock_fd);
    return 0;
}
