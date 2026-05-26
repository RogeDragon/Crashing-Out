
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/epoll.h>
#include "network.h"
#include <stdatomic.h>
#include "debug.h"

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
        pthread_mutex_lock(&(args_val->clients->manager_mutex));

        for (int x = 0; x < args_val->clients->number_items; x++)
        {
            struct client * selected_client = (args_val->clients->struct_array)[x];

            if (kill(selected_client->client_process_id, SIGUSR1) == -1)
            {
                char * message = malloc(strlen("Disconnect") + 1);
                strcpy(message, "Disconnect");
                push_node(args_val->message_queue, "Disconnect", selected_client);

                #if DEBUG
                    fprintf(stderr, "Pushing Disconnect!");
                #endif
            }
        }

        pthread_mutex_unlock(&(args_val->clients->manager_mutex));
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

    #if DEBUG
        fprintf(stderr, "(main) Started Server\n");
        fprintf(stderr, "(main) creating new dynamic variables\n");
    #endif

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

    #if DEBUG
        fprintf(stderr,"(main) finished creating new dynamic variables\n");
        fprintf(stderr,"(main) creating checking thread\n");
    #endif

    pthread_t client_checker;

    struct handel_disconnect_args * args = malloc( sizeof( struct handel_disconnect_args ) );
    args->clients = clients;
    args->message_queue = message_queue;

    pthread_create(&client_checker, NULL, handel_disconnect, (void *) args);

    #if DEBUG
        fprintf(stderr, "(main) finished creating checking thread\n");
    #endif

    #if DEBUG
        fprintf(stderr, "(main) creating thread pool instances\n");
    #endif

    struct threadpool * threadpool;
    intialise_threadpool( atoi(argv[1]) , &threadpool, message_queue, clients, canvas_manager, sprites, placements, buffer);

    #if DEBUG
        fprintf(stderr, "(main) finished creating thread pool instances\n");
    #endif

    //running the server
    int server_pid = getpid();
    printf("Server: %d.\n", server_pid);
    fflush(stdout);

    #if DEBUG
        fprintf(stderr, "(main) started singal handeling generation\n");
    #endif

    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_flags     = SA_SIGINFO;
    action.sa_sigaction = &handel_new_user;

    sigaction(SIGUSR1, &action, NULL);

    #if DEBUG
        fprintf(stderr,"(main) finished signal handling generation\n");
        fprintf(stderr,"(main) Creating polling data\n");
    #endif

    int monitor = epoll_create1(0);
    struct epoll_event events[MAX_EVENTS];

    int  reading_fd;
    FILE * files[2];
    

    while (1)
    {
        switch (state)
        {
            case wait:  
                #if DEBUG
                    fprintf(stderr, "(wait) waiting for packets to arrive\n");
                #endif

                int n = epoll_wait(monitor, events, MAX_EVENTS, -1);
                if (n == -1) {
                    continue;
                }

                #if DEBUG
                    fprintf(stderr, "(wait) new packet has arrived!\n");
                #endif

            
                for (int i = 0; i < n; i++)
                {
                    for (int x = 0; x < get_number_items(clients); x++)
                    {
                        #if DEBUG
                            fprintf(stderr, "(wait) Looping through new packets\n");
                        #endif

                        struct client * selected_client;
                        get_item(clients, x, (void **) &selected_client);

                        #if DEBUG
                            fprintf(stderr, "(wait) Selected Player\n");
                        #endif
                        
                        if (selected_client->reading_fd == events[i].data.fd)
                        {
                            char buffer[100];
                            ssize_t bytes = read(selected_client->reading_fd, buffer, sizeof(buffer) - 1);

                            if (bytes <= 0)
                            {
                                push_node_message_queue(message_queue, "Disconnect", selected_client);
                                continue;
                            }

                            char * message = (char *) malloc(strlen(buffer) + 1);
                            strcpy(message, buffer);

                            #if DEBUG
                                fprintf(stderr, "(wait) starting new node push\n");
                                fprintf(stderr, "(wait) push message %p\n", message);
                                fprintf(stderr, "(wait) push message q %p\n", message_queue);
                                fprintf(stderr, "(wait) push client %p\n", selected_client);
                            #endif

                            push_node_message_queue(message_queue, message, selected_client);
                        }
                    }   
                }
            break;

            case connect:
                #if DEBUG
                    fprintf(stderr, "(connect) connecting a new client to server!\n");
                    fprintf(stderr, "(connect) PID: %d\n", pid);
                    fprintf(stderr, "(connect) reading_fd: %d\n", reading_fd);
                #endif

                create_pipes(pid);
                kill(pid, SIGUSR2);
                open_pipes_server(files, pid, &reading_fd);
                state = login;

                #if DEBUG
                    fprintf(stderr, "(connect) finish connecting a client to the server!\n");
                    fprintf(stderr, "(connect) reading_fd: %d\n", reading_fd);
                #endif
            break;

            case login:
                #if DEBUG
                    fprintf(stderr, "(login) started loggin\n");
                #endif

                char line[256];
                ssize_t bytes = read(fileno(files[1]), line, sizeof(line) - 1);

                if (bytes <= 0)
                {
                    state = disconnect;
                    continue;
                }

                char command[128];
                char argument[128];
                sscanf(line, "%s %s", command, argument);

                if ( strcmp("Login", command) == 0 )
                {
                    int status = check_user_login(argument, "users.txt");
                    if (status == 1)
                    {
                        #if DEBUG
                            fprintf(stderr, "(login) new client has loggedin!\n");
                        #endif

                        fprintf(files[0], "%d\n", status);
                        fflush(files[0]);

                        struct client * new_client;
                        create_client(&new_client, reading_fd, files[1], files[0], pid);
                        push_dynamic_manager(clients, (void *) new_client);

                        #if DEBUG
                            fprintf(stderr, "(login) new client has loggedin finished!\n");
                        #endif

                        struct epoll_event event;
                        event.events = EPOLLIN;
                        event.data.fd = reading_fd;
                        epoll_ctl(monitor, EPOLL_CTL_ADD, reading_fd, &event);

                        #if DEBUG
                            fprintf(stderr, "(connect) epoll has been registered\n");
                        #endif

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
                #if DEBUG
                    fprintf(stderr, "(disconnect) disconnecting the new client!\n");
                #endif

                kill(pid, SIGUSR1);
                close_and_unlink_pipes(files, pid);
                pid = 0;
                reading_fd = 0;
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
