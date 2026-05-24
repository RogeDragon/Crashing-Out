#include "buffer.h"

/*************************************************************************
    Packet Functions
-------------------------------------------------------------------------*/

void create_packet(struct packet ** new_packet, char * message, int packet_id, struct client * owner)
{
    *new_packet = (struct packet *) malloc(sizeof(struct packet));

    (*new_packet)->owner     = owner;
    (*new_packet)->packet_id = packet_id;
    (*new_packet)->message   =  (char *) malloc(strlen(message) + 1);

    strncpy((*new_packet)->message, message, strlen(message) + 1 );
}

void destroy_packet(struct packet * packet)
{
    free(packet->message);
    free(packet);
}

/*************************************************************************
    Output Manager Functions
-------------------------------------------------------------------------*/

void create_buffer (struct buffer ** new_buffer)
{
    struct buffer * buffer = (struct buffer *) malloc( sizeof(buffer));
    dynamic_manager_init(&(buffer->packet_array));
    sem_init(&(buffer->avaliable), 0, 0);
    *new_buffer = buffer;
}

void destroy_buffer(struct buffer * buffer)
{
    for (int i = 0; i < get_number_items(buffer->packet_array) ; i++) //find the canvas that has this one it!
    {
        struct packet * value;
        get_item(buffer->packet_array, i, (void **) &value);        
        destroy_packet(value);
    }

    pthread_mutex_destroy( &(buffer->packet_array->manager_mutex) );
    sem_destroy(&(buffer->avaliable));

    free(buffer->packet_array->struct_array);
    free(buffer->packet_array);
    free(buffer);

    printf("The output manager has been deleted!\n");
}

void push_packet (struct buffer * buffer, char * message, int packet_id, struct client * owner)
{
    struct packet * new_packet;
    create_packet(&new_packet, message, packet_id, owner);
    push_dynamic_manager(buffer->packet_array, new_packet);
    sem_post(&(buffer->avaliable));
}

struct packet * pop_avaliable_packets(struct dynamic_manager * clients, struct buffer * buffer)
{
    sem_wait(&buffer->avaliable);

    for (int i = 0; i < get_number_items(clients); i++)
    {
        struct client * client;
        get_item(clients, i, (void **) &client);

        for (int i = 0; i < get_number_items(buffer->packet_array); i++)
        {
            struct packet * value;
            get_item(buffer->packet_array, i, (void **) &value);    

            if (value->packet_id == get_next_packet_id(client) && value->owner == client)
            {
                struct packet * returned_value;
                pop_dynamic_manager_item(buffer->packet_array, (void **) &returned_value, i);

                return returned_value;
            }
        }
    }
    return NULL;
}

struct packet * get_packet(struct buffer * buffer, int index)
{
    struct packet * returned_packet;
    get_item(buffer->packet_array, index, (void **) &returned_packet);
    return returned_packet;
}

/*************************************************************************
    Output Manager Test Code!
-------------------------------------------------------------------------*/
/*
int main()
{
    struct dynamic_manager * clients;
    dynamic_manager_init(&clients);
    
    struct buffer * buffer;
    create_buffer(&buffer);

    struct client * client;
    create_client(&client, 100, 100);
    push_dynamic_manager(clients, (void *) client);

    push_packet(buffer, "Hello World!", get_avaliable_id(client), client);
    struct packet * popped_packet = pop_avaliable_packets(clients, buffer);

    destroy_packet(popped_packet);

    destroy_buffer(buffer);

    return 0;
}
*/

/*
int main()
{
    struct buffer * buffer;
    create_buffer(&buffer);
    destroy_buffer(buffer);

    return 0;
}
*/