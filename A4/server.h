#ifndef SERVER_H
#define SERVER_H

#define MAXNAME 30
#define BUFSIZE 30

struct buf_reader {
    char *buf;
    int inbuf;
    int room;
    char *after;
};

/*
 * state: 1: waiting for name
 *        2: waiting for role
 *        3: waiting for instruction
 *        5: student enter course
 *        (4 is aborted from using for design reason!)
 */ 
struct client {
    int sock_fd;
    char role; // This should be either T or S
    int state;
    char name[MAXNAME];
    struct buf_reader buf;
    struct client *next;
};

typedef struct client Client;
typedef struct buf_reader Reader;


int find_network_newline(const char *buf, int n);
void rm_client(Client **lst, Client *client, fd_set *all_fds);
void init_reader(Reader *reader);
Client *find_stu_client(Client *clients, char *name);

#endif