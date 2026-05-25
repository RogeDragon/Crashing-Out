#ifndef NETWORK_H
#define NETWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signalfd.h>

#define SERVER 0
#define CLIENT 1
#define MAX_EVENTS 64

void make_pipes( int * fds, int process_id, int type);
bool check_user_login (char * username, char * file_path, int * return_balance);
void add_file_descriptor (int epoll_fd, int file_descriptor);

#endif