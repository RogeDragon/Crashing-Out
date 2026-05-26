
#ifndef COMMAND_H
#define COMMAND_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "manager.h"
#include "struct.h"
#include "buffer.h"
#include "client.h"
#include <stdbool.h>


bool check_and_convert_arguement(char * arg, int * result);
void sort_command(char * raw_command, struct client * client, struct dynamic_manager * clients, struct dynamic_manager * canvas_manager, struct dynamic_manager * sprite_manager, struct dynamic_manager * placement_manager, struct buffer * output_buffer);
void execute_command(char * instruction, char ** arguments, struct client * client, struct dynamic_manager * clients, struct dynamic_manager * canvas_manager, struct dynamic_manager * sprite_manager, struct dynamic_manager * placement_manager, struct buffer * output_buffer);

#endif
