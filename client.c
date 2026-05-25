#include "client.h"
#include "struct.h"
#include <stdio.h>

/*************************************************************************
    Client Functions
-------------------------------------------------------------------------*/

void create_client(struct client ** returned_client, int reading_fd, int writing_fd)
{
    struct client * client = (struct client *) malloc( sizeof(struct client) );
    client->avaliable_id = 0;
    client->next_packet_id = 0;

    client->reading = fdopen(reading_fd, "r");
    client->reading_fd = reading_fd;
    client->writing_fd = writing_fd;

    dynamic_manager_init(&client->sprites);
    dynamic_manager_init(&client->placements);
    dynamic_manager_init(&client->shared_canvas);

    pthread_mutex_init(&client->client_lock, NULL);
    *returned_client = client;
}

void destroy_client (struct client * client)
{
    //freeing the placements
    for (int i = 0; i < get_number_items(client->placements); i++) //find the canvas that has this one it!
    {
        struct modified_placement * value;
        get_item(client->placements, i, (void **) &value);        
        destroy_modified_placement(value);
    }

    pthread_mutex_destroy( &(client->placements->manager_mutex) );
    free(client->placements->struct_array);
    free(client->placements);

    //freeing the sprites
    for (int i = 0; i < get_number_items(client->sprites); i++)
    {
        struct sprite * value;
        get_item(client->sprites, i, (void **) &value);
        animate_destroy_sprite(value);
    }

    pthread_mutex_destroy( &(client->sprites->manager_mutex) );
    free(client->sprites->struct_array);
    free(client->sprites);

    //freeing the canvas (make a thread safe canvas)
    for (int i = 0; i < get_number_items(client->shared_canvas); i++)
    {
        struct thread_safe_canvas * value;
        get_item(client->shared_canvas, i, (void **) &value);
        try_destory_thread_safe_canvas(value);
    }

    //freeing the struct
    pthread_mutex_destroy( &(client->client_lock) );
    free(client);
}

int get_avaliable_id(struct client * client)
{
    pthread_mutex_lock( &(client->client_lock) );
    int avaliable_id = client->avaliable_id++;
    pthread_mutex_unlock( &(client->client_lock) );

    return avaliable_id;
}

int get_next_packet_id(struct client * client)
{
    pthread_mutex_lock( &(client->client_lock) );
    int next_id = client->next_packet_id;
    pthread_mutex_unlock( &(client->client_lock) );

    return next_id;
}

void increment_next_packet_id(struct client * client)
{
    pthread_mutex_lock( &(client->client_lock) );
    client->next_packet_id++;
    pthread_mutex_unlock( &(client->client_lock) );
}

/*************************************************************************
    Client Testing Code!
-------------------------------------------------------------------------*/

/*
int main()
{
    struct client * test_client;
    create_client(&test_client, 1, 1);

    printf("Avaliable ID: %d\n", get_avaliable_id(test_client));
    printf("Avaliable ID: %d\n", get_avaliable_id(test_client));

    printf("Next ID: %d\n", get_next_packet_id(test_client));
    increment_next_packet_id(test_client);
    printf("Next ID: %d\n", get_next_packet_id(test_client));

    destroy_client(test_client);
    return 0;
}
*/