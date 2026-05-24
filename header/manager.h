#ifndef MANAGER_H
#define MANAGER_H

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
    Dynamic Manager Member Functions
-------------------------------------------------------------------------*/

int dynamic_manager_init (struct dynamic_manager ** dynamic_manager);
int push_dynamic_manager (struct dynamic_manager * dynamic_manager, void * new_value);
int pop_dynamic_manager_item(struct dynamic_manager * dynamic_manager, void ** returned_value, int index);
int check_dynamic_manager (struct dynamic_manager * dynamic_manager, void * selected_item);
int find_selected_item(struct dynamic_manager * dynamic_manager, void * selected_item, void ** returned_value);
int pop_selected_item(struct dynamic_manager * dynamic_manager, void * selected_item, void ** returned_value);
void get_item(struct dynamic_manager * dynamic_manager, int index, void ** returned_value);
int get_number_items(struct dynamic_manager * dynamic_manager);
int get_capacity_items(struct dynamic_manager * dynamic_manager);


#endif