#include "network.h"
#include "client.h"
#include "manager.h"
#include "message.h"
#include "buffer.h"
#include <stdio.h>
#include <pthread.h>


struct output_thread_data 
{
    struct buffer *          buffer;
    struct dynamic_manager * clients;
};

pthread_t output_thread;

void * manage_output_buffer(void * arg)
{
    struct output_thread_data * data   = (struct output_thread_data *) arg;
    struct buffer * buffer             = data->buffer;
    struct dynamic_manager * clients   = data->clients;

    while (1)
    {
        struct packet * popped_packet = pop_avaliable_packets(clients, buffer);

        size_t message_length = strlen(popped_packet->message);
        if (message_length == 0 || popped_packet->message[message_length - 1] != '\n')
        {
            char * outgoing_message = (char *) malloc(message_length + 2);
            if (outgoing_message == NULL)
            {
                perror("malloc");
                destroy_packet(popped_packet);
                continue;
            }

            memcpy(outgoing_message, popped_packet->message, message_length);
            outgoing_message[message_length] = '\n';
            outgoing_message[message_length + 1] = '\0';

            write(popped_packet->owner->writing_fd, outgoing_message, message_length + 1);
            free(outgoing_message);
        }
        else
        {
            write(popped_packet->owner->writing_fd, popped_packet->message, message_length);
        }

        destroy_packet(popped_packet);
    }
}

int main(int argc, char ** argv)
{
    if (argc != 2) return 1;

    //Making all the variables!, unholy abombination
    struct dynamic_manager * clients;
    dynamic_manager_init(&clients);

    struct dynamic_manager * sprites;
    dynamic_manager_init(&sprites);

    struct dynamic_manager * placements;
    dynamic_manager_init(&placements);

    struct dynamic_manager * canvas_manager;
    dynamic_manager_init(&canvas_manager);

    struct message_queue * new_message_queue;
    create_message_queue(&new_message_queue);
     
    struct buffer * buffer;
    create_buffer(&buffer);
 
    //setting up the server
    int server_pid = getpid();
    printf("Server PID: %d.\n", server_pid);

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);
    sigprocmask(SIG_BLOCK, &mask, NULL);
    
    int signal_fd = signalfd(-1, &mask, FNONBLOCK | O_CLOEXEC);
    int file_monitor = epoll_create1(0);

    add_file_descriptor(file_monitor, signal_fd);

    struct threadpool * threadpool;
    intialise_threadpool( atoi(argv[1]) , &threadpool, new_message_queue, canvas_manager, sprites, placements, buffer);

    struct output_thread_data data = 
    {
        .buffer  = buffer,
        .clients = clients
    };

    pthread_create(&output_thread, NULL, &manage_output_buffer, (void *) &data); 

    while (1)
    {
        struct epoll_event ready_events[MAX_EVENTS];
        int n = epoll_wait(file_monitor, ready_events, MAX_EVENTS, -1);

        for (int i = 0; i < n; i ++)
        {
            if (ready_events[i].data.fd == signal_fd)
            {
                if (ready_events[i].events & EPOLLIN)
                {
                    struct signalfd_siginfo info;
                    read(signal_fd, &info, sizeof(info));
                    kill(info.ssi_pid, SIGUSR2);

                    int fds[2];
                    make_pipes(fds, info.ssi_pid ,SERVER);

                    add_file_descriptor(file_monitor, fds[0]);

                    struct client * client;
                    create_client(&client, fds[0], fds[1]);
                    push_dynamic_manager(clients, (void *) client);
                }
            }
            else if (ready_events[i].events & EPOLLIN)
            {
                for (int x = 0; x < get_number_items(clients); x++)
                {
                    struct client * selected_client;
                    get_item(clients, x, (void **) &selected_client);

                    if (selected_client->reading_fd == ready_events[i].data.fd)
                    {
                        char * command = NULL;
                        size_t length = 0;
                        getline(&command, &length, selected_client->reading);
                        push_node_message_queue(new_message_queue, command, selected_client);
                    }
                }
            }
        }
    }

    return 0;
}