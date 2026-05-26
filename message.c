
#include "message.h"

/*************************************************************************
    Message Queue 'Member Functions'
-------------------------------------------------------------------------*/

void create_message_queue(struct message_queue ** new_message_queue)
{
    *new_message_queue = (struct message_queue *) malloc(sizeof(struct message_queue));

    (*new_message_queue)->head = NULL;
    (*new_message_queue)->tail = NULL;
    (*new_message_queue)->size = 0;

    pthread_mutex_init(&((*new_message_queue)->message_queue_lock), NULL);
    pthread_cond_init(&((*new_message_queue)->queue_condition), NULL);

    //printf("Message Created\n");
}

void push_node_message_queue(struct message_queue * selected_message_queue, char * value, struct client * client)
{
    pthread_mutex_lock( &(selected_message_queue->message_queue_lock) );

    struct node * new_node = (struct node *) malloc( sizeof( struct node ) );
    new_node->next         = NULL;
    new_node->previous     = selected_message_queue->tail;
    
    new_node->client       = client;
    new_node->message      = value;
    
    selected_message_queue->size++;
    selected_message_queue->tail = new_node;

    if (selected_message_queue->head == NULL) {
        selected_message_queue->head = new_node;
    }

    pthread_cond_signal( &(selected_message_queue->queue_condition) );
    pthread_mutex_unlock( &(selected_message_queue->message_queue_lock) );

    //printf("Message Pushed\n");
}

void pop_node_message_queue(struct message_queue * selected_message_queue, struct node ** returned_node)
{

    pthread_mutex_lock( &(selected_message_queue->message_queue_lock) );
    while ( selected_message_queue->size == 0 )
    {
        pthread_cond_wait( &(selected_message_queue->queue_condition), &(selected_message_queue->message_queue_lock) );
    }

    //printf("Message Popped\n");

    //printf("Ptr Dereference Start\n");
    *returned_node = selected_message_queue->head;
    //printf("Ptr Dereference End\n");

    if (selected_message_queue->head != NULL) {
        selected_message_queue->head = selected_message_queue->head->next;

        (*returned_node)->next = NULL;
        (*returned_node)->previous = NULL;

        selected_message_queue->size--;
    }

    //printf("Message Popped\n");

    pthread_mutex_unlock( &(selected_message_queue->message_queue_lock) );
}

/*************************************************************************
    Thread Pool Handler Function
-------------------------------------------------------------------------*/

void * worker_thread(void * arg)
{
    struct threadpool * provided_threadpool = (struct threadpool * ) arg;
    struct message_queue * message_queue = provided_threadpool->message_queue;

    struct dynamic_manager * clients           = provided_threadpool->clients;
    struct dynamic_manager * canvas_manager    = provided_threadpool->canvas_manager;
    struct dynamic_manager * sprite_manager    = provided_threadpool->sprite_manager;
    struct dynamic_manager * placement_manager = provided_threadpool->placement_manager;

    struct buffer * buffer = provided_threadpool->buffer;

    while (1)
    {        
        struct node * returned_node;
        pop_node_message_queue(message_queue, &returned_node); // this waits the thread with a condition variable!

        sort_command(returned_node->message, returned_node->client, clients,  canvas_manager, sprite_manager, placement_manager, buffer);
        
        free(returned_node->message);
        free(returned_node);
    }
}

/*************************************************************************
    Thread Pool 'Member functions'
-------------------------------------------------------------------------*/

int intialise_threadpool(int number_threads, struct threadpool ** threadpool, 
                        struct message_queue * message_queue,
                        struct dynamic_manager * canvas_manager,
                        struct dynamic_manager * client_manager,
                        struct dynamic_manager * sprite_manager,
                        struct dynamic_manager * placement_manager,
                        struct buffer * buffer)
{
    *threadpool = (struct threadpool *) malloc( sizeof(struct threadpool) );
    (*threadpool)->number_threads    = number_threads;
    (*threadpool)->message_queue     = message_queue;
    (*threadpool)->clients           = client_manager;
    (*threadpool)->canvas_manager    = canvas_manager;
    (*threadpool)->sprite_manager    = sprite_manager;
    (*threadpool)->placement_manager = placement_manager;
    (*threadpool)->buffer            = buffer;

    (*threadpool)->threadpool_threads = (pthread_t *) malloc( sizeof(pthread_t) * number_threads);

    for (int i = 0; i < number_threads; i++)
    {
        pthread_create(&((*threadpool)->threadpool_threads[i]), NULL, &worker_thread, (void *) (*threadpool) );
    }

    return 0;
}

int join_threadpool(struct threadpool * threadpool)
{
    for (int i = 0; i < threadpool->number_threads; i++)
    {
        pthread_join(threadpool->threadpool_threads[i], NULL);
    }

    free(threadpool->threadpool_threads);
    free(threadpool);
    return 0;
}

/*************************************************************************
   Message Queue Testing functions + Testing Code
-------------------------------------------------------------------------*/

/*
void * testing_producer( void * raw_message_queue_ptr )
{
    struct message_queue * message_queue_ptr = (struct message_queue *) raw_message_queue_ptr;

    for (int i = 0; i < 10; i++)
    {
        char * x = malloc(sizeof(char) * 2);
        x[0] = i;
        x[1] = '\0';

        push_node_message_queue(message_queue_ptr, x);
    }

    printf("Test: Finshed Consumer Test \n");
}


void * testing_consumer( void * raw_message_queue_ptr )
{
    struct message_queue * message_queue_ptr = (struct message_queue *) raw_message_queue_ptr;

    for (int i = 0; i < 10; i++)
    {
        struct node * returned_node;
        pop_node_message_queue(message_queue_ptr, &returned_node);

        free(returned_node->message);
        free(returned_node);

        returned_node = NULL;
    }

    printf("Test: Finshed Producer Test \n");
}

int main()
{
    struct message_queue * new_message_queue;
    create_message_queue(&new_message_queue);

    pthread_t consumer_thread;
    pthread_t producer_thread;

    pthread_create(&producer_thread, NULL, &testing_producer, (void *) new_message_queue);
    pthread_create(&consumer_thread, NULL, &testing_consumer, (void *) new_message_queue);

    pthread_join(consumer_thread, NULL);
    pthread_join(producer_thread, NULL);

    return 0;
}
*/

/*************************************************************************
   Thread Pool Testing Functions + Testing Code
-------------------------------------------------------------------------*/
/*
struct sample_producer_struct
{
    struct client * client;
    struct message_queue * message_queue_ptr;
};

void * testing_threadpool_producer( void * raw_producer_struct )
{
    struct sample_producer_struct * struct_producer = (struct sample_producer_struct *) raw_producer_struct;
    struct message_queue * message_queue_ptr = struct_producer->message_queue_ptr;
    struct client        * client = struct_producer->client;

    for (int i = 0; i < 10; i++)
    {
        char message[]      = "create_circle 10 000010 1";
        char * node_message = (char *) malloc(strlen(message));
        strcpy(node_message, message);

        push_node_message_queue(message_queue_ptr, node_message, client);
        printf("PUSH!\n");
    }

    printf("Test: Finshed Consumer Test \n");
}

int main()
{   
    struct dynamic_manager * clients;
    dynamic_manager_init(&clients);

    struct dynamic_manager * sprites;
    dynamic_manager_init(&sprites);

    struct dynamic_manager * placements;
    dynamic_manager_init(&placements);

    struct dynamic_manager * canvas_manager;
    dynamic_manager_init(&canvas_manager);

    struct buffer * buffer;
    create_buffer(&buffer);

    struct client * client;
    create_client(&client, 100, 100);
    push_dynamic_manager(clients, (void *) client);

    struct message_queue * new_message_queue;
    create_message_queue(&new_message_queue);

    struct threadpool * threadpool;
    intialise_threadpool(4, &threadpool, new_message_queue, canvas_manager, sprites, placements, buffer);

    struct sample_producer_struct sample = {.message_queue_ptr = new_message_queue, .client = client};

    pthread_t threadpool_procducer;
    pthread_create(&threadpool_procducer, NULL, &testing_threadpool_producer, (void *) &sample);

    join_threadpool(threadpool);
    return 0;
}
*/
