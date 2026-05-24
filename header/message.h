#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdatomic.h>
#include "command.h"
#include "manager.h"
#include "buffer.h"


/*************************************************************************
    Message Queue Structs
-------------------------------------------------------------------------*/

struct node
{
    char   *        message;
    struct node *   next;
    struct node *   previous;
    struct client * client;
};

struct message_queue
{
    struct node *   head;
    struct node *   tail;

    pthread_mutex_t head_lock;
    pthread_mutex_t tail_lock;

    pthread_cond_t  queue_condition;
};

/*************************************************************************
    Message Queue 'Member Functions'
-------------------------------------------------------------------------*/

int create_message_queue(struct message_queue ** new_message_queue);
int push_node_message_queue(struct message_queue * selected_message_queue, char * value, struct client * client);
int pop_node_message_queue(struct message_queue * selected_message_queue, struct node ** returned_node);
int destroy_message_queue(struct message_queue * selected_message_queue);

/*************************************************************************
    Thread Pool Structs
-------------------------------------------------------------------------*/

struct threadpool
{
    int number_threads;
    pthread_t * threadpool_threads;
    struct message_queue * message_queue;

    struct dynamic_manager * canvas_manager;
    struct dynamic_manager * sprite_manager;
    struct dynamic_manager * placement_manager;

    struct buffer * buffer;
};

/*************************************************************************
    Thread Pool Functions
-------------------------------------------------------------------------*/

void * worker_thread(void * arg);
int intialise_threadpool(int number_threads, struct threadpool ** threadpool, 
                        struct message_queue * message_queue,
                        struct dynamic_manager * canvas_manager, 
                        struct dynamic_manager * sprite_manager,
                        struct dynamic_manager * placement_manager,
                        struct buffer * buffer);