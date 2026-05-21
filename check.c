#include <animate/animate.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "manager.h"
#include "check.h"

struct canvas * convert_canvas (int64_t value)
{
    return (struct canvas *) value;
}

struct sprite_placement * convert_placement(int64_t value)
{
    return (struct sprite_placement *) value;
}

bool verify_number(int lower, int maximum, int value)
{
    if (value >= lower && value <= maximum)
    {
        return true; 
    }
    else
    {
        return false;
    }

}

bool check_command_argument(enum argument_types arguement_type, struct dynamic_manager * canvas_manager, struct dynamic_manager * placement_manager, struct dynamic_manager * sprite_manager, char * value)
{
    switch (arguement_type)
    {
        case FILE_VALUE:
            return (access(value, F_OK) != -1);
        break;

        case BOOL:
            if (!strcmp(value, "0") || !strcmp(value, "1")) return true;
            else return false;
        break;

        case COLOUR:
            color_t colour_value = strtoul(value, NULL, 16);
            if ( colour_value >  strtoul("FFFFFF", NULL, 16)) return false;
        break;

        case SPRITE:
            return !check_dynamic_manager(canvas_manager, (void *) value);
        break;

        case CANVAS:
            return !check_dynamic_manager(canvas_manager, (void *) value);
        break;

        case SPRITE_PLACEMENT:
            return !check_dynamic_manager(canvas_manager, (void *) value);
        break;

        default:
            return true;
        break;
    }
}

/*
int main()
{
    struct canvas * canvas_value = animate_create_canvas(100, 100, 0);
    struct dynamic_manager  * canvas_manager;

    dynamic_manager_init(&canvas_manager);
    push_dynamic_manager(canvas_manager, canvas_value);

    struct sprite           * sprite_value  = animate_create_circle(10, 0, true);
    struct dynamic_manager  * sprite_manager;

    dynamic_manager_init(&sprite_manager);
    push_dynamic_manager(sprite_manager, (void *) sprite_value);

    struct sprite_placement * placed_sprite = animate_place_sprite(canvas_value, sprite_value, 100, 100);
    struct dynamic_manager  * placement_manager;

    dynamic_manager_init(&placement_manager);
    push_dynamic_manager(placement_manager, (void *) placed_sprite);


    char * canvas_ptr = (char *) canvas_value;
    enum argument_types example_arugment = CANVAS;
    if (check_command_argument(example_arugment, canvas_manager, placement_manager, sprite_manager, canvas_ptr))
    {
        printf("SUCCESS: The arguement checker works!\n");
    }
    else
    {
        printf("FAILED: The argument checker does not work!\n");
    }

    if (check_command_argument(example_arugment, canvas_manager, placement_manager, sprite_manager, NULL))
    {
        printf("FAILED: The argument checker does not work!\n");
    }
    else
    {
        printf("SUCCESS: The arguement checker works!\n");
    }

    return 0;
}
*/
