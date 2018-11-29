#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "server.h"

/*
 * Search the first n characters of buf for a network newline (\r\n).
 * Return one plus the index of the '\n' of the first network newline,
 * or -1 if no network newline is found.
 */
int find_network_newline(const char *buf, int n) {
    int i = 0;
    while (i < n - 1) {
        if (buf[i] == '\r' && buf[i+1] == '\n') {
            return i + 2;
        }
        i++;
    }
    return -1;
}

/*
 * Remove client from linked list of client lst and free the associated
 * memory.
 */
void rm_client(Client **lst, Client *client, fd_set *all_fds) {
    Client *cur = *lst;
    FD_CLR(client->sock_fd, all_fds);
    close(client->sock_fd);
    if (cur->sock_fd == client->sock_fd) {
        *lst = cur->next;
        free(cur->buf.buf);
        free(cur);
        return;
    }
    while (cur->next != NULL) {
        if (cur->next->sock_fd == client->sock_fd) {
            Client *tofree = cur->next;
            cur->next = cur->next->next;
            free(cur->buf.buf);
            free(tofree);
            return;
        }
        cur = cur->next;
    }
}

/*
 * Initilize the buffered reader.
 */
void init_reader(Reader *reader) {
    reader->buf = malloc(sizeof(char) * (BUFSIZE + 2));
    memset(reader->buf, '\0', BUFSIZE + 2);
    reader->inbuf = 0;
    // Need to include 2 for \r\n !
    reader->room = BUFSIZE + 2;
    reader->after = reader->buf;
}

/*
 * Return a pointre to the client with given name. Return NULL on not found.
 */ 
Client *find_client(Client *clients, char *name) {
    Client *cur = clients;
    while (cur != NULL) {
        if (strcmp(cur->name, name) == 0) {
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}


