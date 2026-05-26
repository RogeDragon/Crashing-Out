#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdatomic.h>
#include "command.h"
#include "manager.h"
#include "buffer.h"
#include <signal.h>


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
    sig_atomic_t size;
    struct node *   head;
    struct node *   tail;

    pthread_mutex_t message_queue_lock;
    pthread_cond_t  queue_condition;
};

/*************************************************************************
    Message Queue 'Member Functions'
-------------------------------------------------------------------------*/

void create_message_queue(struct message_queue ** new_message_queue);
void push_node_message_queue(struct message_queue * selected_message_queue, char * value, struct client * client);
void pop_node_message_queue(struct message_queue * selected_message_queue, struct node ** returned_node);
void push_node(struct message_queue * selected_message_queue, char * value, struct client * client);

/*************************************************************************
    Thread Pool Structs
-------------------------------------------------------------------------*/

struct threadpool
{
    int number_threads;
    pthread_t * threadpool_threads;
    struct message_queue * message_queue;

    struct dynamic_manager * clients;
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
                        struct dynamic_manager * client_manager,
                        struct dynamic_manager * canvas_manager, 
                        struct dynamic_manager * sprite_manager,
                        struct dynamic_manager * placement_manager,
                        struct buffer * buffer);