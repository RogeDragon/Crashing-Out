#ifndef  NET_H
#define NET_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

void create_pipes(int process_id);
void open_pipes_client(FILE **FILES, int process_id);
void open_pipes_server(FILE **FILES, int process_id, int * reading_fd);
int check_user_login(char *username, char *file_path);
void close_and_unlink_pipes(FILE **FILES, int process_id);
void close_pipes(FILE **FILES, int process_id);

#endif