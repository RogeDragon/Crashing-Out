#ifndef STRUCT_H
#define STRUCT_H

#include <pthread.h>
#include <animate/animate.h>
#include <stdlib.h>
#include <stdbool.h>
#include "manager.h"
#include "client.h"

struct thread_safe_canvas
{
    int             number_references;
    struct canvas * canvas;
    pthread_mutex_t canvas_mutex;
};

struct modified_placement
{
    struct client * owner;
    struct thread_safe_canvas * placement_canvas;
    struct sprite_placement   * placement;
};

struct modified_sprite
{
    struct client * owner;
    struct sprite * sprite;
};


void create_thread_safe_canvas ( struct thread_safe_canvas ** canvas, size_t width, size_t hieght, color_t colour);
void destory_thread_safe_canvas( struct thread_safe_canvas * canvas );
void try_destory_thread_safe_canvas( struct thread_safe_canvas * canvas );
bool non_decrement_try_destory_thread_safe_canvas(struct thread_safe_canvas * canvas);
int  get_number_references();

void create_modified_placement(struct modified_placement ** placement, struct client * client, struct thread_safe_canvas * canvas, struct modified_sprite * sprite, ssize_t x, ssize_t y);
void destroy_modified_placement(struct modified_placement * placement);

void create_modified_sprite(struct modified_sprite ** m_sprite, struct client * client, struct sprite * sprite);
void destroy_modified_sprite(struct modified_sprite * sprite);

#endif