#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdlib.h>
#include "network.h"
#include "manager.h"
#include "message.h"
#include "buffer.h"

enum connection_state {
    waiting,
    connected
};

static volatile enum connection_state state = waiting;
static volatile siginfo_t signal_info;

void signal_handler(int signum, siginfo_t* info, void * context) 
{
    (void) context;

    if (signum == SIGUSR1)
    {
        signal_info = *info;
        state = connected;
    }
}

int main(int argc, char** argv) {
    if (argc != 2) return 1;

    int server_pid = getpid();
    printf("Server PID: %d.\n", server_pid);

    struct sigaction action;
    action.sa_sigaction = &signal_handler;
    action.sa_flags = SA_SIGINFO;
    sigemptyset(&action.sa_mask);

    sigaction(SIGUSR1, &action, NULL);
    sigaction(SIGUSR2, &action, NULL);

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

    int polly = epoll_create1(0);

    struct threadpool * threadpool;
    intialise_threadpool( atoi(argv[1]) , &threadpool, message_queue, canvas_manager, sprites, placements, buffer);

    pthread_t output_thread;

    struct output_thread_data data = 
    {
        .buffer  = buffer,
        .clients = clients
    };
    
    pthread_create(&output_thread, NULL, &manage_output_buffer, (void *) &data); 

    while (1)
    {
        switch (state)
        {
            case waiting:
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
            break;

            case connected:
                int fds[2];
                kill(signal_info.si_pid, SIGUSR2);
                make_pipes(fds, signal_info.si_pid ,SERVER);

                struct epoll_event event;
                event.events = EPOLLIN;
                event.data.fd = fds[0];
                epoll_ctl(polly, EPOLL_CTL_ADD, fds[0], &event);

                struct epoll_event ready;
                epoll_wait(polly, &ready, 1, -1);

                if (ready.events & EPOLLIN)
                {
                    char buffer[100];
                    read(fds[0], buffer, sizeof(buffer));

                    printf("%s", buffer);

                    char * instruction = strtok(buffer, " ");
                    (void) instruction; // the client make sures the instruction is right!

                    char * name = strtok(NULL, " ");
                    printf("%s", name);

                    int balance;
                    if ( check_user_login(name, "users.txt", &balance) )
                    {
                        char balance_array[20];
                        sprintf(balance_array, "%d", balance);

                        write(fds[1], balance_array, 20);
                        write(fds[1], "\n", 1);

                        struct client * new_client;
                        create_client(&new_client,  fds[0],  fds[1]);
                        push_dynamic_manager(clients, (void *) new_client);
                        state = waiting;
                    }
                    else 
                    {
                        if (balance < 0)
                        {
                            write(fds[1], "-1", 1);
                            write(fds[1], "\n", 1);
                        }
                        else
                        {
                            write(fds[1], "-2", 1);
                            write(fds[1], "\n", 1);
                        }

                        close(fds[0]);
                        close(fds[1]);

                        sleep(1);
                        char c2s[64];
                        snprintf(c2s, sizeof(c2s), "./FIFO_C2S_%d", signal_info.si_pid);

                        char s2c[64];
                        snprintf(s2c, sizeof(s2c), "./FIFO_S2C_%d", signal_info.si_pid);

                        unlink(c2s);
                        unlink(s2c);
                    }
                    
                }
            break;
        }
    }

    return 0;
}
