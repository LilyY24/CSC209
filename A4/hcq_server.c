#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "hcq.h"
#include "server.h"

#ifndef PORT
    #define PORT 50271
#endif

#define MAX_BACKLOG 5
#define WELCOME "Welcome to the Help Centre, what is your name?\r\n"
#define INVALID_ROLE "Invalid role (enter T or S)\r\n"
//TODO: Ask about this
#define VALID_TA "Valid commands for TA:\r\n        stats\r\n        next\r\n        (or use Ctrl-C to leave)\r\n"
#define VALID_STU "You have been entered into the queue. While you wait, you can use the command stats to see which TAs are currently serving students.\r\n"
#define WRONG_SYN "Incorrect syntax\r\n"
#define WHICH_COURSE "Valid courses: CSC108, CSC148, CSC209\r\nWhich course are you asking about?\r\n"
#define INVALID_COURSE "This is not a valid course. Good-bye.\r\n"
#define TA_OR_STU "Are you a TA or a Student (enter T or S)?\r\n"
#define STU_TAKE "Your turn to see the TA.\r\nWe are disconnecting you from the server now. Press Ctrl-C to close nc\r\n"
#define ALREADY_IN "You are already in the queue and cannot be added again for any course. Good-bye.\r\n"
//TODO: Change this!


//TODO: Need to close socket upon remove!
// Use global variables.
Ta *ta_list = NULL;
Student *stu_list = NULL;
Client *clients = NULL;
Course *courses;
int num_courses = 3;

/*
 * Initialize courses list.
 */ 
void prepare_courses() {
    if ((courses = malloc(sizeof(Course) * 3)) == NULL) {
        perror("malloc for course list\n");
        exit(1);
    }
    strcpy(courses[0].code, "CSC108");
    strcpy(courses[1].code, "CSC148");
    strcpy(courses[2].code, "CSC209");
}

/*
 * Read from the client. Return 1 when they have entered a whole command
 * and save the command in array instruction. Return -1 if they type more than
 * 30 character consecutively without a new line(which is a illegal input),
 *  return 4 if they are disconnecting from the server. Return 0 if no command
 * is yet entered.
 */ 
// TODO: still need to handle the case there the username is exactly 30 char long.
int read_from(Client *client, char *instruction) {
    Reader *reader = &client->buf;
    int nbytes = read(client->sock_fd, reader->after, reader->room);
    if (nbytes == 0) {
        return 4;
    }
    if (nbytes < 0) {
        perror("read from client");
        exit(1);
    }
    reader->inbuf += nbytes;
    int where = find_network_newline(reader->buf, reader->inbuf);
    if (where > 0) {
        reader->buf[where - 2] = '\0';
        strcpy(instruction, reader->buf);
        memmove(reader->buf, reader->buf+where, BUFSIZE + 2 - where);
        reader->inbuf -= where;
        reader->after = reader->buf + reader->inbuf;
        reader->room = (BUFSIZE + 2) - reader->inbuf;
        return 1;
    } else if (reader->inbuf > BUFSIZE) {
        // Enter more than 30 char and have no \r\n, means illegal input!
        return -1;
    }
    reader->after = reader->buf + reader->inbuf;
    reader->room = BUFSIZE + 2 - reader->inbuf;
    return 0;
}

/*
 * Handle the case where client is entering his/her name. And change her/his
 * state to state2, where his role is asked. Write to the client the prompt 
 * information to ask the role of client.
 */ 
void handle_state1(Client *client, char *instruction) {
    strcpy(client->name, instruction);
    client->state = 2;
    int nbytes = write(client->sock_fd, TA_OR_STU, strlen(TA_OR_STU));
    if (nbytes != strlen(TA_OR_STU)) {
        perror("write to client");
        exit(1);
    }
}

/*
 * Handle the case there client is entering its role in the help center, where 
 * only T or S is valid. Change the role of the client and change the 
 * client's state, write the client the valid command or ask which course
 * they are asking when the input is valid. 
 * Ask the client to re-enter its role when input is not valid.
 */
void handle_state2(Client *client, char *instruction) {
    if (strcmp(instruction, "T") == 0 || strcmp(instruction, "S") == 0) {
        client->role = instruction[0];
        int wbytes = 0;
        if (instruction[0] == 'T') {
            add_ta(&ta_list, client->name);
            wbytes = write(client->sock_fd, VALID_TA, strlen(VALID_TA));
            if (wbytes != strlen(VALID_TA)) {
                perror("Write to client");
                exit(1);
            }
            client->state = 3;
        } else {
            wbytes = write(client->sock_fd, WHICH_COURSE, strlen(WHICH_COURSE));
            client->state = 5;
            if (wbytes != strlen(WHICH_COURSE)) {
                perror("Write to client");
                exit(1);
            }
        }
    } else {
        int wbytes = write(client->sock_fd, INVALID_ROLE, strlen(INVALID_ROLE));
        if (wbytes != strlen(INVALID_ROLE)) {
            perror("write to client");
            exit(1);
        }
    }
}

/*
 * Handle the case where the client is good to give instructions.
 */
void handle_state3(Client *client, char *instruction, Client **clients, fd_set *all_fds) {
    int wbytes;
    if (client->role == 'T') {
        if (strcmp(instruction, "stats") == 0) {
            char *result = print_full_queue(stu_list);
            wbytes = write(client->sock_fd, result, strlen(result));
            if (wbytes != strlen(result)) {
                perror("write to client");
                exit(1);
            }
            free(result);
        } else if (strcmp(instruction, "next") == 0) {
            if (next_overall(client->name, &ta_list, &stu_list) == 1) {
                // Should be impossible to reach here!
                fprintf(stderr, "Something bad happen! Ta not in the list!");
                exit(1);
            }
            Ta *this_ta = find_ta(ta_list, client->name);
            if (this_ta->current_student != NULL) {
                char *name_tofree = this_ta->current_student->name;
                Client *tofree = find_stu_client(*clients, name_tofree);
                if (tofree == NULL) {
                    //It should be impossible to reach here
                    fprintf(stderr, "Something bad happen, Student is not a client");
                    exit(1);
                }
                wbytes = write(tofree->sock_fd, STU_TAKE, strlen(STU_TAKE));
                if (wbytes != strlen(STU_TAKE)) {
                    perror("write to client");
                    exit(1);
                }
                rm_client(clients, tofree, all_fds);
            }
        } else {
            wbytes = write(client->sock_fd, WRONG_SYN, strlen(WRONG_SYN));
            if (wbytes != strlen(WRONG_SYN)) {
                perror("write to client");
                exit(1);
            }
        }
    } else {
        if (strcmp(instruction, "stats") == 0) {
            char *result = print_currently_serving(ta_list);
            wbytes = write(client->sock_fd, result, strlen(result));
            if (wbytes != strlen(result)) {
                perror("write to client");
                exit(1);
            }
            free(result);
        } else {
            wbytes = write(client->sock_fd, WRONG_SYN, strlen(WRONG_SYN));
            if (wbytes != strlen(WRONG_SYN)) {
                perror("write to client");
                exit(1);
            }
        }
    }
}

/*
 * Handle the case where a student is asked which course he/she want to ask.
 * Disconnect the client upon invalid course. Return -1 on student already in 
 * the queue., return 0 on normal.
 */ 
int handle_state5(Client *client, char *instruction, fd_set *all_fds) {
    int wbytes;
    if (strcmp(instruction, "CSC108") == 0 ||
        strcmp(instruction, "CSC209") == 0 ||
        strcmp(instruction, "CSC148") == 0) {
            int status = add_student(&stu_list, client->name, instruction, 
                                        courses, num_courses);
            if (status == 0) {
                wbytes = write(client->sock_fd, VALID_STU, strlen(VALID_STU));
                if (wbytes != strlen(VALID_STU)) {
                    perror("write to client");
                    exit(1);
                }
                client->state = 3;
            } else if (status == 1) {
                wbytes = write(client->sock_fd, ALREADY_IN, strlen(ALREADY_IN));
                if (wbytes != strlen(ALREADY_IN)) {
                    perror("write to client");
                    exit(1);
                }
                return -1;
            } else {
                // Should never be able to reach here, since we already checked.
                fprintf(stderr, "Course not in the list\n");
                exit(1);
            }
    } else {
        wbytes = write(client->sock_fd, INVALID_COURSE, strlen(INVALID_COURSE));
        if (wbytes != strlen(INVALID_COURSE)) {
            perror("write to client");
            exit(1);
        }
        rm_client(&clients, client, all_fds);
    }
    return 0;  
}

int main(){
    prepare_courses();
    
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("Server: socket");
        exit(1);
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port= htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;
    memset(&server.sin_zero, 0, 8);

    int on = 1;
    int status = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR,
                            (const char *) &on, sizeof(on));
    if(status == -1) {
        perror("setsockopt -- REUSEADDR");
    }

    if (bind(sock_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("server: bind");
        close(sock_fd);
        exit(1);
    }

    if (listen(sock_fd, MAX_BACKLOG) < 0) {
        perror("server: listen");
        close(sock_fd);
        exit(1);
    }

    int max_fd = sock_fd;
    fd_set all_fds;
    FD_ZERO(&all_fds);
    FD_SET(sock_fd, &all_fds);
    int nwrite;

    while (1) {
        fd_set listen_fds = all_fds;
        int nready = select(max_fd + 1, &listen_fds, NULL, NULL, NULL);
        if (nready == -1) {
            perror("server: select");
            exit(1);
        }

        if (FD_ISSET(sock_fd, &listen_fds)) {
            Client *new_client = malloc(sizeof(Client));
            if (new_client == NULL) {
                perror("malloc for new_client");
                exit(1);
            }
            new_client->next = NULL;
            init_reader(&new_client->buf);
            int client_fd = accept(sock_fd, NULL, NULL);
            if (client_fd < 0) {
                perror("server: accept");
                close(sock_fd);
                exit(1);
            }
            new_client->sock_fd = client_fd;
            //TODO: null terminated? IS the add 1 necessary?
            nwrite = write(client_fd, WELCOME, strlen(WELCOME) + 1);
            if (nwrite != strlen(WELCOME) + 1) {
                perror("Write to client");
                close(sock_fd);
                exit(1);
            }
            new_client->state = 1;
            FD_SET(client_fd, &all_fds);
            if (client_fd > max_fd) {
                max_fd = client_fd;
            }

            if (clients == NULL) {
                clients = new_client;
            } else {
                new_client->next = clients;
                clients = new_client;
            }
        }

        Client *cur = clients;
        while (cur != NULL) {
            // This variable record whether cur has already been updated.
            int is_updated = 0;
            if (FD_ISSET(cur->sock_fd, &listen_fds)) {
                char *instruction = malloc(1024 * sizeof(char));
                if (instruction == NULL) {
                    perror("malloc for instruction");
                    exit(1);
                }
                memset(instruction, '\0', 1024);
                int result = read_from(cur, instruction);
                if (result == -1 || result == 4) {
                    if (cur->state == 3) {
                        if (cur->role == 'T') {
                            remove_ta(&ta_list, cur->name);
                        } else {
                            give_up_waiting(&stu_list, cur->name);
                        }
                    }
                    // Since we need to remove this client, need to update 
                    // cur here.
                    Client *temp = cur;
                    cur = cur->next;
                    is_updated = 1;
                    rm_client(&clients, temp, &all_fds);
                } else if (result == 1){
                    if (cur->state == 1) {
                        handle_state1(cur, instruction);
                    } else if (cur->state == 2) {
                        handle_state2(cur, instruction);
                    } else if (cur->state == 3) {
                        handle_state3(cur, instruction, &clients, &all_fds);
                    } else if (cur->state == 5) {
                        int status = handle_state5(cur, instruction, &all_fds);
                        if (status == -1) {
                            Client *temp = cur;
                            cur = cur->next;
                            is_updated = 1;
                            rm_client(&clients, temp, &all_fds);
                        }
                    }
                }
                free(instruction);
            }
            if (is_updated == 0) {
                cur = cur->next;
            }
        }
    }
}