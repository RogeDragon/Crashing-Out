/**************************************************************************
 * FILE: manager.c
 * 
 * PURPOSE: This is a implementation of the dynamic
 * array A.D.T. This manager 'object' is to be used with
 * 'paser.c'. It is used to check if a client provided
 * 'ID' matched with available ID's.
 * 
 * AUTHOR: 550704212
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include "manager.h"
#include "debug.h"


/*************************************************************************
    Dynamic Manager 'Member Functions'
-------------------------------------------------------------------------*/

int dynamic_manager_init (struct dynamic_manager ** dynamic_manager)
{ 
    *dynamic_manager = (struct dynamic_manager *) malloc( sizeof( struct dynamic_manager ) );

    if (*dynamic_manager == NULL)
    {
        return -2;
    }

    (*dynamic_manager)->number_items   = 0;
    (*dynamic_manager)->capacity_items = 4;
    (*dynamic_manager)->struct_array    = (void *) calloc( (*dynamic_manager)->capacity_items ,  sizeof(void *));
    
    pthread_mutex_init(&(*dynamic_manager)->manager_mutex, NULL);

    if ((*dynamic_manager)->struct_array == NULL)
    {
        pthread_mutex_destroy(&(*dynamic_manager)->manager_mutex);
        free(*dynamic_manager);
        return 1;
    }

    return 0;
}

int push_dynamic_manager (struct dynamic_manager * dynamic_manager, void * new_value)
{
    pthread_mutex_lock(&(dynamic_manager->manager_mutex));

    if (dynamic_manager->capacity_items == dynamic_manager->number_items)
    {
        dynamic_manager->capacity_items = dynamic_manager->capacity_items * 2;
        dynamic_manager->struct_array    = (void *) realloc(dynamic_manager->struct_array, dynamic_manager->capacity_items * sizeof(void *));

        if (dynamic_manager->struct_array == NULL)
        {
            pthread_mutex_unlock(&(dynamic_manager->manager_mutex));
            return 1;
        }
    }

    dynamic_manager->struct_array[dynamic_manager->number_items++] = new_value;
    pthread_mutex_unlock(&(dynamic_manager->manager_mutex));
    
    return 0;
}

int pop_dynamic_manager_item(struct dynamic_manager * dynamic_manager, void ** returned_value, int index)
{
    pthread_mutex_lock( &(dynamic_manager->manager_mutex) );

    if (dynamic_manager->number_items <= (dynamic_manager->capacity_items/4) ) 
    {
        dynamic_manager->capacity_items = dynamic_manager->capacity_items/2;
        dynamic_manager->struct_array    = (void *) realloc(dynamic_manager->struct_array, dynamic_manager->capacity_items * sizeof(void *));

        if (dynamic_manager->struct_array == NULL) 
        {
            pthread_mutex_unlock( &(dynamic_manager->manager_mutex) );
            return 1;
        }
    }

    *returned_value = dynamic_manager->struct_array[index];

    for (int x = index + 1; x < dynamic_manager->number_items - 1; x++)
    {
        dynamic_manager->struct_array[x] = dynamic_manager->struct_array[x + 1];
    }

    dynamic_manager->number_items--;
    pthread_mutex_unlock( &(dynamic_manager->manager_mutex) );

    return 0;
}

int check_dynamic_manager (struct dynamic_manager * dynamic_manager, void * selected_item)
{
    pthread_mutex_lock( &(dynamic_manager->manager_mutex) );

    for (int x = 0; x < dynamic_manager->number_items; x++)
    {
        void * current_item = (dynamic_manager->struct_array)[x];
        if (current_item == selected_item)
        {
            pthread_mutex_unlock( &(dynamic_manager->manager_mutex) );
            return 0;
        }
    }

    pthread_mutex_unlock( &(dynamic_manager->manager_mutex) );
    return 1;
}

int find_selected_item(struct dynamic_manager * dynamic_manager, void * selected_item, void ** returned_value)
{
    pthread_mutex_lock( &(dynamic_manager->manager_mutex) );
    for (int x = 0; x < dynamic_manager->number_items; x++)
    {
        void * current_item = (dynamic_manager->struct_array)[x];
        if (current_item == selected_item)
        {
            *returned_value = current_item;
            pthread_mutex_unlock( &(dynamic_manager->manager_mutex) );
            return 0;
        }     

    }
    pthread_mutex_unlock( &(dynamic_manager->manager_mutex) );
    return 1;
}

static int pop_item_unlocked(struct dynamic_manager * dynamic_manager, void ** returned_value, int index)
{
    *returned_value = dynamic_manager->struct_array[index];
    for (int x = index; x < dynamic_manager->number_items - 1; x++)
    {
        dynamic_manager->struct_array[x] = dynamic_manager->struct_array[x + 1];
    }
    dynamic_manager->number_items--;
    return 0;
}

int pop_selected_item(struct dynamic_manager * dynamic_manager, void * selected_item, void ** returned_value)
{
    pthread_mutex_lock(&dynamic_manager->manager_mutex);
    for (int x = 0; x < dynamic_manager->number_items; x++)
    {
        if (dynamic_manager->struct_array[x] == selected_item)
        {
            pop_item_unlocked(dynamic_manager, returned_value, x);
            pthread_mutex_unlock(&dynamic_manager->manager_mutex);
            return 0;
        }
    }
    pthread_mutex_unlock(&dynamic_manager->manager_mutex);
    return 1;
}

void get_item(struct dynamic_manager * dynamic_manager, int index, void ** returned_value)
{
    pthread_mutex_lock( &(dynamic_manager->manager_mutex) );
    *returned_value = dynamic_manager->struct_array[index]; 
    pthread_mutex_unlock( &(dynamic_manager->manager_mutex) ); 
}

int get_number_items(struct dynamic_manager * dynamic_manager)
{
    pthread_mutex_lock( &(dynamic_manager->manager_mutex) );
    int number_items = dynamic_manager->number_items;
    pthread_mutex_unlock( &(dynamic_manager->manager_mutex) ); 

    return number_items;
}

int get_capacity_items(struct dynamic_manager * dynamic_manager)
{
    pthread_mutex_lock( &(dynamic_manager->manager_mutex) );
    int capacity = dynamic_manager->capacity_items;
    pthread_mutex_unlock( &(dynamic_manager->manager_mutex) ); 

    return capacity;
}


/*************************************************************************
    Testing Functions + Testing Structs
-------------------------------------------------------------------------*/
/*
int main()
{
    struct dynamic_manager * dynamic_manager;
    dynamic_manager_init(&dynamic_manager);

    char * value = "EXAMPLE TEST";
    
    for (int i = 0; i < 20; i++) 
    {
        push_dynamic_manager(dynamic_manager, (void *) value);
    }

    char * popped_value;
    for (int i = 0; i < 20; i++) 
    {
        pop_dynamic_manager_item(dynamic_manager, (void **) &popped_value, dynamic_manager->number_items - 1);
    }
    return 0;
}
*/