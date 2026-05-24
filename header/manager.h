#ifndef MANAGER_H
#define MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

struct dynamic_manager;

int dynamic_manager_init (struct dynamic_manager ** dynamic_manager);
int push_dynamic_manager (struct dynamic_manager * dynamic_manager, void * new_value);
void destory_dynamic_manager(struct dynamic_manager * dynamic_manager);
int pop_dynamic_manager_item(struct dynamic_manager * dynamic_manager, void ** returned_value, int index);
int check_dynamic_manager (struct dynamic_manager * dynamic_manager, void * selected_item);

#endif