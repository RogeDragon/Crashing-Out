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

/*************************************************************************
    Dynamic Manager Structs
-------------------------------------------------------------------------*/

struct dynamic_manager
{
    int              number_items;
    int              capacity_items;
    void     **      struct_array;
    
    pthread_mutex_t  manager_mutex;
};

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

    printf("Canvas Manager: Canvas Manger Created\n");

    return 0;
}

int push_dynamic_manager (struct dynamic_manager * dynamic_manager, void * new_value)
{
    pthread_mutex_lock(&(dynamic_manager->manager_mutex));

    if (dynamic_manager->capacity_items == dynamic_manager->number_items)
    {
        dynamic_manager->capacity_items = dynamic_manager->capacity_items * 2;
        dynamic_manager->struct_array    = (void *) realloc(dynamic_manager->struct_array, dynamic_manager->capacity_items * sizeof(void *));

        printf("Resize: Canvas Array Has Been Resized \n");

        if (dynamic_manager->struct_array == NULL)
        {
            pthread_mutex_unlock(&(dynamic_manager->manager_mutex));
            return 1;
        }
    }

    dynamic_manager->struct_array[dynamic_manager->number_items++] = new_value;
    pthread_mutex_unlock(&(dynamic_manager->manager_mutex));

    printf("Added: Canvas Has Been Added \n");
    
    return 0;
}

void destory_dynamic_manager(struct dynamic_manager * dynamic_manager)
{
    for (int i = 0; i < dynamic_manager->number_items; i++)
    {
        free(dynamic_manager->struct_array[i]);
    }

    pthread_mutex_destroy( &(dynamic_manager->manager_mutex) );

    free(dynamic_manager->struct_array);
    free(dynamic_manager);
}

int pop_dynamic_manager_item(struct dynamic_manager * dynamic_manager, void ** returned_value, int index)
{
    pthread_mutex_lock( &(dynamic_manager->manager_mutex) );

    if (dynamic_manager->number_items <= (dynamic_manager->capacity_items/4) ) 
    {
        dynamic_manager->capacity_items = dynamic_manager->capacity_items/2;
        dynamic_manager->struct_array    = (void *) realloc(dynamic_manager->struct_array, dynamic_manager->capacity_items * sizeof(void *));

        printf("Resize: Canvas Array Has Been Resized \n");

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

    printf("Popped: Canvas Has Been Popped \n");
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