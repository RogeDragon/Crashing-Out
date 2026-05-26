#include "buffer.h"
#include <unistd.h>
#include "debug.h"

/*************************************************************************
    Packet Functions
-------------------------------------------------------------------------*/

void create_packet(struct packet ** new_packet, char * message, int packet_id, struct client * owner)
{
    *new_packet = (struct packet *) malloc(sizeof(struct packet));

    (*new_packet)->owner     = owner;
    (*new_packet)->packet_id = packet_id;
    (*new_packet)->message   = (char *) malloc(strlen(message) + 1);

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
    struct buffer * buffer = (struct buffer *) malloc( sizeof(struct buffer));
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
}

void push_packet (struct buffer * buffer, char * message, int packet_id, struct client * owner)
{
    #if DEBUG
        printf("(push packet) starting to push new packet!\n");
        printf("(push packet) created new packet\n");
    #endif

    struct packet * new_packet;
    create_packet(&new_packet, message, packet_id, owner);

    #if DEBUG
        printf("(push packet) finished making new packet\n");
    #endif

    push_dynamic_manager(buffer->packet_array, new_packet);

    #if DEBUG
        printf("(push packet) finished push new packet!\n");
        printf("(push packet) starting sem post\n");
    #endif

    sem_post(&(buffer->avaliable));

    #if DEBUG
        printf("(push packet) finishing sem post\n");
    #endif
}

struct packet * pop_avaliable_packets(struct dynamic_manager * clients, struct buffer * buffer)
{
    #if DEBUG
        fprintf(stderr, "(pop available) started waiting for outputs\n");
    #endif

    sem_wait(&buffer->avaliable);

    #if DEBUG
        fprintf(stderr, "(pop available) begin popping outputs\n");
    #endif

    for (int i = 0; i < get_number_items(clients); i++)
    {
        #if DEBUG
            fprintf(stderr, "(pop available) start selecting a client\n");
        #endif

        struct client * client;
        get_item(clients, i, (void **) &client);

        #if DEBUG
            fprintf(stderr, "(pop available) finish selecting a client\n");
        #endif

        for (int i = 0; i < get_number_items(buffer->packet_array); i++)
        {
            struct packet * value;
            get_item(buffer->packet_array, i, (void **) &value);   
    
            #if DEBUG
                fprintf(stderr, "(pop available) check if client matches\n");
            #endif

            if (value->packet_id == get_next_packet_id(client) && value->owner == client)
            {
                struct packet * returned_value;
                pop_dynamic_manager_item(buffer->packet_array, (void **) &returned_value, i);

                return returned_value;
            }

            #if DEBUG
                fprintf(stderr, "(pop available) check if client matches\n");
            #endif
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

void * manage_output_buffer(void * arg)
{
    struct output_thread_data * data   = (struct output_thread_data *) arg;
    struct buffer * buffer             = data->buffer;
    struct dynamic_manager * clients   = data->clients;

    while (1)
    {
        struct packet * popped_packet = pop_avaliable_packets(clients, buffer);
        if (popped_packet == NULL) continue;
        
        fprintf(popped_packet->owner->writing, "%s\n", popped_packet->message);
        destroy_packet(popped_packet);
    }
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