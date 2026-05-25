#ifndef BUFFER_H
#define BUFFER_H

#include "manager.h"
#include <pthread.h>
#include "client.h"
#include <string.h>
#include <stdlib.h>
#include <semaphore.h>


/*************************************************************************
    Packet + Buffer Struct
-------------------------------------------------------------------------*/

struct packet
{
    int             packet_id;
    char *          message;
    struct client * owner;
};

struct buffer
{
    struct dynamic_manager * packet_array;
    sem_t avaliable;
};

struct output_thread_data 
{
    struct buffer *          buffer;
    struct dynamic_manager * clients;
};

/*************************************************************************
    Packet + Buffer Functions
-------------------------------------------------------------------------*/

void create_packet(struct packet ** new_packet, char * message, int packet_id, struct client * owner);
void destroy_packet(struct packet * packet);
void create_buffer (struct buffer ** new_buffer);
void destroy_buffer(struct buffer * buffer);
void push_packet (struct buffer * buffer, char * message, int packet_id, struct client * owner);
struct packet * pop_avaliable_packets(struct dynamic_manager * clients, struct buffer * buffer);
struct packet * get_packet(struct buffer * buffer, int index);
void * manage_output_buffer(void * arg);

#endif