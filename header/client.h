
#ifndef CLIENT_H
#define CLIENT_H

#include <animate/animate.h>
#include "manager.h"
#include <stdlib.h>

enum client_state
{
    LOGIN,
    RUNNING,
    DISCONNECT
};

struct client
{
    pthread_mutex_t client_lock; //this is a lock for the id!
    int avaliable_id; // a monotonic increasing counter
    int next_packet_id;

    int writing_fd;
    int reading_fd;
    FILE * reading;

    struct dynamic_manager * sprites;
    struct dynamic_manager * placements;
    struct dynamic_manager * shared_canvas;
};

void create_client(struct client ** returned_client, int reading_fd, int writing_fd);
void destroy_client (struct client * client); //destroy file descriptor as well!
int get_avaliable_id(struct client * client);
int get_next_packet_id(struct client * client);

#endif