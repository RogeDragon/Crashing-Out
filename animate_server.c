
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/epoll.h>
#include "network.h"
#include <stdatomic.h>

#include "manager.h"
#include "buffer.h"
#include "struct.h"
#include "client.h"
#include "message.h"

#define MAX_EVENTS 256

enum server_states
{
    wait,
    connect,
    login,
    disconnect
};

volatile sig_atomic_t state = wait;
volatile sig_atomic_t pid   = 0;

struct handel_disconnect_args 
{
    struct dynamic_manager * clients;
    struct message_queue * message_queue;
};

void * handel_disconnect(void *args)
{
    struct handel_disconnect_args * args_val = (struct handel_disconnect_args *) args;

    while (1)
    {
        for (int x = 0; x < get_number_items(args_val->clients); x++)
        {
            struct client * selected_client;
            get_item(args_val->clients, x, (void **) &selected_client);

            if (kill(selected_client->client_process_id, SIGUSR1) == -1)
            {
                push_node_message_queue(args_val->message_queue, "Disconnect", selected_client);
            }
        }
        sleep(1);
    }
}

void handel_new_user(int signal, siginfo_t * info, void * context)
{
    if (signal == SIGUSR1)
    {
        state = connect;
        pid   = info->si_pid;
        (void) context;
    }
}

int main (int argc, char ** argv)
{
    if (argc != 2) return 1;

    //intialising variables
    struct dynamic_manager * clients;
    dynamic_manager_init(&clients);

    struct dynamic_manager * sprites;
    dynamic_manager_init(&sprites);

    struct dynamic_manager * placements;
    dynamic_manager_init(&placements);

    struct dynamic_manager * canvas_manager;
    dynamic_manager_init(&canvas_manager);

    struct message_queue * message_queue;
    create_message_queue(&message_queue);
        
    struct buffer * buffer;
    create_buffer(&buffer);

    pthread_t client_checker;

    struct handel_disconnect_args * args = malloc( sizeof( struct handel_disconnect_args ) );
    args->clients = clients;
    args->message_queue = message_queue;

    pthread_create(&client_checker, NULL, handel_disconnect, (void *) args);

    struct threadpool * threadpool;
    intialise_threadpool( atoi(argv[1]) , &threadpool, message_queue, clients, canvas_manager, sprites, placements, buffer);

    //running the server
    int server_pid = getpid();
    printf("Server: %d.\n", server_pid);
    fflush(stdout);

    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_flags     = SA_SIGINFO;
    action.sa_sigaction = &handel_new_user;

    sigaction(SIGUSR1, &action, NULL);

    int monitor = epoll_create1(0);
    struct epoll_event events[MAX_EVENTS];

    int  reading_fd;
    FILE * files[2];

    while (1)
    {
        switch (state)
        {
            case wait:    
                int n = epoll_wait(monitor, events, MAX_EVENTS, -1);
                if (n == -1) continue;
            
                for (int i = 0; i < n; i++)
                {
                    for (int x = 0; x < get_number_items(clients); x++)
                    {
                        struct client * selected_client;
                        get_item(clients, x, (void **) &selected_client);
                        
                        if (selected_client->reading_fd == events[i].data.fd)
                        {
                            char buffer[100];
                            fgets(buffer, sizeof(buffer), selected_client->reading);

                            char * message = (char *) malloc(strlen(buffer));
                            strcpy(message, buffer);
                            
                            push_node_message_queue(message_queue, message, selected_client);
                        }
                    }   
                }
            break;

            case connect:
                create_pipes(pid);
                kill(pid, SIGUSR2);
                open_pipes_server(files, pid, &reading_fd);

                struct epoll_event event;
                event.events = EPOLLIN;
                event.data.fd = reading_fd;
                epoll_ctl(monitor, EPOLL_CTL_ADD, reading_fd, &event);

                state = login;
            break;

            case login:
                char line[256];
                fgets(line, 256, files[1]);

                char command[128];
                char argument[128];
                sscanf(line, "%s %s", command, argument);

                if ( strcmp("Login", command) == 0 )
                {
                    int status = check_user_login(argument, "users.txt");
                    if (status == 1)
                    {
                        fprintf(files[0], "%d\n", status);
                        fflush(files[0]);

                        struct client * new_client;
                        create_client(&new_client, reading_fd, files[1], files[0], pid);
                        push_dynamic_manager(clients, (void *) new_client);

                        state = wait;
                        continue;
                    }
                    else if (status == -1)
                    {
                        fprintf(files[0], "-1\n");
                        fflush(files[0]);

                        sleep(1);
                        state = disconnect;
                        continue;
                    }
                    else
                    {
                        fprintf(files[0], "-2\n");
                        fflush(files[0]);

                        sleep(1);
                        state = disconnect;
                        continue;;
                    }
                }
                else 
                {
                    state = disconnect;
                    continue;
                }
            break;

            case disconnect:
                close_and_unlink_pipes(files, pid);
                state = wait;
            break;
        }
    }
    return 0;
}

/*
struct epoll_event ready_events[MAX_EVENTS];
int n = epoll_wait(polly, ready_events, MAX_EVENTS, -1);

if (n == -1)
{
    continue;
}

for (int i = 0; i < n; i++)
{
    for (int x = 0; x < get_number_items(clients); x++)
    {
        struct client * selected_client;
        get_item(clients, x, (void **) &selected_client);
        
        if (selected_client->reading_fd == ready_events[i].data.fd)
        {
            char buffer[100];
            fgets(buffer, sizeof(buffer), selected_client->reading);
            read(selected_client->reading_fd, buffer, 100);

            char * message = (char *) malloc(strlen(buffer));
            strcpy(message, buffer);
            
            push_node_message_queue(message_queue, message, selected_client);
        }
    }   
}
*/
