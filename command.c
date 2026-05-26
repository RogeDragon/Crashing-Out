
#include "command.h"
#include "network.h"
#include "debug.h"

bool check_and_convert_arguement(char * arg, int * result)
{
    if (arg == NULL)
    {
        return true;
    }

    char * value_error;
    *result = strtol(arg, &value_error, 10);

    if (*value_error != '\0')
    {
        return true;
    }
    return false;
}

#define NUM_ARGS 5

void sort_command(char * raw_command, struct client * client,  struct dynamic_manager * clients, struct dynamic_manager * canvas_manager, struct dynamic_manager * sprite_manager, struct dynamic_manager * placement_manager, struct buffer * output_buffer)
{
    char * instruction = strtok(raw_command, " ");
    char * arguments[NUM_ARGS];

    for (int i = 0; i < NUM_ARGS; i ++)
    {
        arguments[i] = strtok(NULL, " ");
    }

    execute_command( instruction, arguments, client, clients, canvas_manager, sprite_manager, placement_manager, output_buffer);
}

void execute_command(char * instruction, char ** arguments, struct client * client, struct dynamic_manager * clients, struct dynamic_manager * canvas_manager, struct dynamic_manager * sprite_manager, struct dynamic_manager * placement_manager, struct buffer * output_buffer)
{
    #if DEBUG
    printf("(execute command): %s\n", instruction);
    #endif

    if (strcmp(instruction, "create_canvas") == 0)
    {
        #if DEBUG
            fprintf(stderr, "(create canvas): checking witdth\n");
            fprintf(stderr, "(create canvas): %s\n", arguments[0]);
        #endif

        int width;
        if (check_and_convert_arguement(arguments[0], &width))
        {
            push_packet(output_buffer, "-2", get_avaliable_id(client), client);
            return;
        }

        #if DEBUG
            fprintf(stderr, "(create canvas): checking height\n");
            fprintf(stderr, "(create canvas): %s\n", arguments[1]);
        #endif

        int height;
        if (check_and_convert_arguement(arguments[1], &height))
        {
            push_packet(output_buffer, "-2", get_avaliable_id(client), client);
            return;
        }

        #if DEBUG
            fprintf(stderr, "(create canvas): checking colour\n");
            fprintf(stderr, "(create canvas): %s\n", arguments[2]);
        #endif

        int colour;
        if (check_and_convert_arguement(arguments[2], &colour))
        {
            push_packet(output_buffer, "-2", get_avaliable_id(client), client);
            return;
        }

        #if DEBUG
            fprintf(stderr, "(create canvas): begin pushing new canvas\n");
        #endif

        struct thread_safe_canvas * canvas;
        create_thread_safe_canvas(&canvas, width, height, colour);
        push_dynamic_manager(canvas_manager, (void *) canvas);
        push_dynamic_manager(client->shared_canvas, (void *) canvas);

        #if DEBUG
            fprintf(stderr, "(create canvas): finish pushing new canvas\n");
        #endif

        #if DEBUG
            fprintf(stderr, "(create canvas): adding message to output buffer\n");
        #endif

        char message[100];
        sprintf(message, "0 %p", canvas);
        push_packet(output_buffer, message, get_avaliable_id(client), client);

        #if DEBUG
            fprintf(stderr, "(create canvas): finish adding message to output buffer\n");
        #endif
    }
    else if (strcmp(instruction, "destroy_canvas") == 0)
    {

        if (check_dynamic_manager(canvas_manager, (void *) arguments[0])) 
        {
            push_packet(output_buffer, "-2", get_avaliable_id(client), client);
            return;
        }

        struct thread_safe_canvas * selected_canvas;
        pop_selected_item(canvas_manager, (void *) arguments[0], (void **) &selected_canvas);
        bool error = non_decrement_try_destory_thread_safe_canvas(selected_canvas); 

        if (error == true)
        {
            push_packet(output_buffer, "-1", get_avaliable_id(client), client);
        }
        else 
        {
            pop_selected_item(client->shared_canvas, (void *) arguments[0], (void **) &selected_canvas);
            push_packet(output_buffer, "0", get_avaliable_id(client), client);
        }
    }
    else if (strcmp(instruction, "create_sprite" ) == 0)
    {
        if (access(arguments[0], F_OK) != -1)
        {
            push_packet(output_buffer, "-2", get_avaliable_id(client), client);
            return;
        }

        struct sprite *          new_file = animate_create_sprite(arguments[0]);
        struct modified_sprite * new_sprite;
        create_modified_sprite(&new_sprite, client, new_file);

        push_dynamic_manager(sprite_manager, (void *) new_sprite);
        push_dynamic_manager(client->sprites, (void *) new_sprite);

        char message[100];
        sprintf(message, "0 %p", new_sprite);
        push_packet(output_buffer, message, get_avaliable_id(client), client);
    }
    else if (strcmp(instruction, "destroy_sprite" ) == 0)
    {
        if ( (!check_dynamic_manager(canvas_manager, (void *) arguments[0])) == 1) 
        {
            push_packet(output_buffer, "-2", get_avaliable_id(client), client);
            return;
        }

        struct modified_sprite * selected_sprite;
        find_selected_item(sprite_manager, (void *) arguments[0], (void **) &selected_sprite);

        if (selected_sprite->owner != client)
        {
            push_packet(output_buffer, "-1", get_avaliable_id(client), client);
            return;
        }

        pop_selected_item(sprite_manager, (void *) arguments[0], (void **) &selected_sprite);
        destroy_modified_sprite(selected_sprite);

        push_packet(output_buffer, "0", get_avaliable_id(client), client);
    }
    else if (strcmp(instruction, "create_rectangle" ) == 0)
    {
        int width;
        if (check_and_convert_arguement(arguments[0], &width))
        {
            push_packet(output_buffer, "-2", get_avaliable_id(client), client);
            return;
        }

        int height;
        if (check_and_convert_arguement(arguments[1], &height))
        {
            push_packet(output_buffer, "-2", get_avaliable_id(client), client);
            return;
        }

        int colour;
        if (check_and_convert_arguement(arguments[2], &colour))
        {
            push_packet(output_buffer, "-2", get_avaliable_id(client), client);
            return;
        }

        int filled;
        if (check_and_convert_arguement(arguments[3], &filled))
        {
            push_packet(output_buffer, "-2", get_avaliable_id(client), client);
            return;
        }

        struct sprite *          new_reactangle = animate_create_rectangle(width, height, colour, filled);
        struct modified_sprite * new_sprite;
        create_modified_sprite(&new_sprite, client, new_reactangle);

        push_dynamic_manager(sprite_manager, (void *) new_sprite);
        push_dynamic_manager(client->sprites, (void *) new_sprite);

        char message[100];
        sprintf(message, "0 %p", new_sprite);
        push_packet(output_buffer, message, get_avaliable_id(client), client);
    }
    else if (strcmp(instruction, "create_circle" ) == 0)
    {
        int radius;
        if (check_and_convert_arguement(arguments[0], &radius))
        {
            push_packet(output_buffer, "-2", get_avaliable_id(client), client);
            return;
        }

        int colour;
        if (check_and_convert_arguement(arguments[1], &colour))
        {
            push_packet(output_buffer, "-2", get_avaliable_id(client), client);
            return;
        }

        int filled;
        if (check_and_convert_arguement(arguments[2], &filled))
        {
            push_packet(output_buffer, "-2", get_avaliable_id(client), client);
            return;
        }

        struct sprite *          new_circle = animate_create_circle(radius, colour, filled);
        struct modified_sprite * new_sprite;
        create_modified_sprite(&new_sprite, client, new_circle);

        push_dynamic_manager(sprite_manager, (void *) new_sprite);
        push_dynamic_manager(client->sprites, (void *) new_sprite);

        char message[100];
        sprintf(message, "0 %p", new_sprite);
        push_packet(output_buffer, message, get_avaliable_id(client), client);
    }
    else if (strcmp(instruction, "place_sprite" ) == 0)
    {
        if ( (!check_dynamic_manager(canvas_manager, (void *) arguments[0])) == 1) 
        {
            push_packet(output_buffer, "-2", get_avaliable_id(client), client);
            return;
        }
        if ( (!check_dynamic_manager(sprite_manager, (void *) arguments[1])) == 1) 
        {
            push_packet(output_buffer, "-2", get_avaliable_id(client), client);
            return;
        }

        int x;
        if (check_and_convert_arguement(arguments[2], &x))
        {
            push_packet(output_buffer, "-2", get_avaliable_id(client), client);
            return;
        }

        int y;
        if (check_and_convert_arguement(arguments[2], &y))
        {
            push_packet(output_buffer, "-2", get_avaliable_id(client), client);
            return;
        }

        struct thread_safe_canvas * selected_canvas;
        find_selected_item(canvas_manager, (void *) arguments[0], (void **) &selected_canvas);

        struct modified_sprite * selected_sprite;
        find_selected_item(sprite_manager, (void *) arguments[1], (void **) &selected_sprite);

        struct modified_placement * new_placement;
        create_modified_placement(&new_placement, client, selected_canvas, selected_sprite, x, y);

        push_dynamic_manager(placement_manager, (void *) new_placement);
        push_dynamic_manager(client->placements, (void *) new_placement);

        char message[100];
        sprintf(message, "0 %p", new_placement);
        push_packet(output_buffer, message, get_avaliable_id(client), client);
    }
    else if (strcmp(instruction, "placement_up" ) == 0)
    {
        if ( (!check_dynamic_manager(placement_manager, (void *) arguments[0])) == 1) 
        {
            push_packet(output_buffer, "-2", get_avaliable_id(client), client);
            return;
        }

        struct modified_placement * selected_placement;
        find_selected_item(placement_manager, (void *) arguments[0], (void **) &selected_placement);

        if (selected_placement->owner != client)
        {
            push_packet(output_buffer, "-1", get_avaliable_id(client), client);
            return;
        }

        pthread_mutex_lock(&selected_placement->placement_canvas->canvas_mutex);
        animate_placement_up(selected_placement->placement);
        pthread_mutex_unlock(&selected_placement->placement_canvas->canvas_mutex);

        push_packet(output_buffer, "0", get_avaliable_id(client), client);
    }

    else if (strcmp(instruction, "placement_down" ) == 0)
    {
        if ( (!check_dynamic_manager(placement_manager, (void *) arguments[0])) == 1) 
        {
            push_packet(output_buffer, "-2", get_avaliable_id(client), client);
            return;
        }

        struct modified_placement * selected_placement;
        find_selected_item(placement_manager, (void *) arguments[0], (void **) &selected_placement);

        if (selected_placement->owner != client)
        {
            push_packet(output_buffer, "-1", get_avaliable_id(client), client);
            return;
        }

        pthread_mutex_lock(&selected_placement->placement_canvas->canvas_mutex);
        animate_placement_down(selected_placement->placement);
        pthread_mutex_unlock(&selected_placement->placement_canvas->canvas_mutex);

        push_packet(output_buffer, "0", get_avaliable_id(client), client);
    }
    else if (strcmp(instruction, "placement_top" ) == 0)
    {
        if ( (!check_dynamic_manager(placement_manager, (void *) arguments[0])) == 1) 
        {
            push_packet(output_buffer, "-2", get_avaliable_id(client), client);
            return;
        }

        struct modified_placement * selected_placement;
        find_selected_item(placement_manager, (void *) arguments[0], (void **) &selected_placement);

        if (selected_placement->owner != client)
        {
            push_packet(output_buffer, "-1", get_avaliable_id(client), client);
            return;
        }

        pthread_mutex_lock(&selected_placement->placement_canvas->canvas_mutex);
        animate_placement_top(selected_placement->placement);
        pthread_mutex_unlock(&selected_placement->placement_canvas->canvas_mutex);

        push_packet(output_buffer, "0", get_avaliable_id(client), client);
    }
    else if (strcmp(instruction, "placement_bottom" ) == 0)
    {
        if ( (!check_dynamic_manager(placement_manager, (void *) arguments[0])) == 1) 
        {
            push_packet(output_buffer, "-2", get_avaliable_id(client), client);
            return;
        }

        struct modified_placement * selected_placement;
        find_selected_item(placement_manager, (void *) arguments[0], (void **) &selected_placement);

        if (selected_placement->owner != client)
        {
            push_packet(output_buffer, "-1", get_avaliable_id(client), client);
            return;
        }

        pthread_mutex_lock(&selected_placement->placement_canvas->canvas_mutex);
        animate_placement_bottom(selected_placement->placement);
        pthread_mutex_unlock(&selected_placement->placement_canvas->canvas_mutex);

        push_packet(output_buffer, "0", get_avaliable_id(client), client);
    }
    else if (strcmp(instruction, "destroy_placement" ) == 0)
    {

    }
    else if (strcmp(instruction, "Disconnect" ) == 0)
    {
        struct client * selected_client;
        pop_selected_item(clients, client, (void **) &selected_client);
        destroy_client(selected_client);
    }
    else
    {
    }

    #if DEBUG
        printf("(execute command): finished\n");
    #endif
}

/*
int main()
{
    struct dynamic_manager * clients;
    dynamic_manager_init(&clients);

    struct dynamic_manager * sprites;
    dynamic_manager_init(&sprites);

    struct dynamic_manager * placements;
    dynamic_manager_init(&placements);
    
    struct buffer * buffer;
    create_buffer(&buffer);

    struct client * client;
    create_client(&client, 100, 100);
    push_dynamic_manager(clients, (void *) client);

    struct dynamic_manager * canvas_manager;
    dynamic_manager_init(&canvas_manager);

    char command[] = "create_canvas 1 2 23";
    execute_command(command, client, canvas_manager, sprites, placements, buffer);
    return 0;
}
*/