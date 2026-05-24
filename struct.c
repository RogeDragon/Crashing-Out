#include "struct.h"

/*************************************************************************
    Thread-Safe Canvas
-------------------------------------------------------------------------*/

void create_thread_safe_canvas ( struct thread_safe_canvas ** canvas, size_t width, size_t hieght, color_t colour)
{
    *canvas = malloc( sizeof( struct thread_safe_canvas) );
    (*canvas)->number_references = 1;
    (*canvas)->canvas = animate_create_canvas(width, hieght, colour);

    pthread_mutex_init( &((*canvas)->canvas_mutex) , NULL);
}

void destory_thread_safe_canvas( struct thread_safe_canvas * canvas )
{
    pthread_mutex_destroy( &(canvas->canvas_mutex) );
    animate_destroy_canvas(canvas->canvas);
    free(canvas); 
}

void try_destory_thread_safe_canvas( struct thread_safe_canvas * canvas )
{
    if (canvas->number_references < 2)
    {
        destory_thread_safe_canvas(canvas);
        return;
    }
    canvas->number_references--;
}

bool non_decrement_try_destory_thread_safe_canvas(struct thread_safe_canvas * canvas )
{
    if (canvas->number_references < 2)
    {
        destory_thread_safe_canvas(canvas);
        return true;
    }
    return false;
}

int get_number_references(struct thread_safe_canvas * canvas)
{
    pthread_mutex_lock( &(canvas->canvas_mutex) );
    int number_references = canvas->number_references;
    pthread_mutex_lock( &(canvas->canvas_mutex) );

    return number_references;
}

/*************************************************************************
    Modified Placement 'Member' Functions!
-------------------------------------------------------------------------*/ 

void create_modified_placement(struct modified_placement ** placement, struct client * client, struct thread_safe_canvas * canvas, struct modified_sprite * sprite, ssize_t x, ssize_t y)
{
    *placement = (struct modified_placement *) malloc(sizeof( struct modified_placement ));
    (*placement)->placement_canvas = canvas;
    (*placement)->owner = client;

    pthread_mutex_lock( &(canvas->canvas_mutex) );
    struct sprite_placement * sprite_placement = animate_place_sprite(canvas->canvas, sprite->sprite, x, y);
    pthread_mutex_unlock( &(canvas->canvas_mutex) );

    (*placement)->placement = sprite_placement;
}

void destroy_modified_placement(struct modified_placement * placement)
{
    pthread_mutex_lock( &(placement->placement_canvas->canvas_mutex));
    animate_destroy_placement(placement->placement);
    pthread_mutex_unlock( &(placement->placement_canvas->canvas_mutex));
    free(placement);
}

/*************************************************************************
    Modified Sprite 'Member' Functions!
-------------------------------------------------------------------------*/ 

void create_modified_sprite(struct modified_sprite ** m_sprite, struct client * client, struct sprite * sprite)
{
    *m_sprite = (struct modified_sprite *) malloc ( sizeof( struct modified_sprite ) );
    (*m_sprite)->owner = client;
    (*m_sprite)->sprite = sprite;
}

void destroy_modified_sprite(struct modified_sprite * sprite)
{
    animate_destroy_sprite(sprite->sprite);
    free(sprite);
}

/*************************************************************************
    Barrier Tracker!
-------------------------------------------------------------------------*/ 
/*
struct barrier
{    
    struct client *             owner;
    int                         required;

    struct thread_safe_canvas * canvas;
    struct dynamic_manager    * clients_connected;

    pthread_mutex_t             bool_lock;
    bool                        barrier;
};

void create_barrier( struct barrier ** result_barrier, int number_required, struct client * client, struct thread_safe_canvas * canvas)
{
    struct barrier * barrier = (struct barrier *) malloc (sizeof( struct barrier ));

    barrier->owner    = client;
    barrier->barrier  = true;
    barrier->canvas   = canvas;
    barrier->required = canvas->number_references - 1;

    pthread_mutex_init(&(barrier->bool_lock), NULL);
    dynamic_manager_init(&(barrier->clients_connected));

    *result_barrier = barrier;
    return barrier; 
}

void check_barrier_function(struct dynamic_manager * barrier_manager, struct client * provided_client, struct thread_safe_canvas * canvas)
{
    //looping through all the barrier and finding the one that matches
    struct barrier * selected_barrier = NULL;
    for (int i = 0; i < barrier_manager->number_items; i++)
    {
        struct barrier * temp;
        get_item(barrier_manager, i, (void **) &temp);

        if (temp->canvas == canvas)
        {
            selected_barrier = temp;
            break;
        }
    }

    if (selected_barrier != NULL)
    {
        if (!check_dynamic_manager(selected_barrier->clients_connected, (void **) &provided_client))
        {
            return;
        } 

        push_dynamic_manager(selected_barrier->clients_connected, (void **) &provided_client);
        return;
    }
    else
    {
        struct barrier * new_barrier;
        create_barrier(&new_barrier, get_number_references(canvas), provided_client, canvas);
        push_dynamic_manager(barrier_manager, (void **) &new_barrier);\
    }
}

void destroy_barrier(struct barrier * barrier)
{
    free(barrier);
}
*/
/*************************************************************************
    Basic Thread-Safe Canvas Test Code!
-------------------------------------------------------------------------*/

/*
int main()
{
    struct thread_safe_canvas * test_canvas;
    create_thread_safe_canvas(&test_canvas, 100, 100, 100000);
    try_destory_thread_safe_canvas(test_canvas);

    return 0;
}
*/

/*************************************************************************
    Modified Placement Test Code!
-------------------------------------------------------------------------*/
/*
int main()
{
    struct sprite * test_sprite = animate_create_circle(10, 100000, 1);

    struct thread_safe_canvas * test_canvas;
    create_thread_safe_canvas(&test_canvas, 100, 100, 100000);

    struct modified_placement * test_placement;
    create_modified_placement(&test_placement, test_canvas, test_sprite, 20, 20);
    
    destroy_modified_placement(test_placement);
    try_destory_thread_safe_canvas(test_canvas);
    return 0;
}
*/