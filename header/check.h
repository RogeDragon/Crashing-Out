#ifndef CHECK_H
#define CHECK_H

#include <animate/animate.h>
#include <stdbool.h>
#include <stdint.h>

enum argument_types
{
    BOOL,
    COLOUR,
    CANVAS,
    SPRITE,
    NUMBER,
    POINTER,
    SIGNED_NUMBER,
    FILE_VALUE,
    SPRITE_PLACEMENT
};

struct canvas * convert_canvas (int64_t value);
struct sprite_placement * convert_placement(int64_t value);
bool verify_number(int lower, int maximum, int value);
bool check_command_argument(enum argument_types arguement_type, struct dynamic_manager * canvas_manager, struct dynamic_manager * placement_manager, struct dynamic_manager * sprite_manager, char * value);

#endif