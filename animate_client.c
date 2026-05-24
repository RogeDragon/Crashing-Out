#include "network.h"
#include <pthread.h>
#include <string.h>

void * write_thread(void * arg)
{
    int * write_fd = (int *) arg;

    while (1)
    {
        char message[50];
        fgets(message, sizeof(message), stdin);
        write(*write_fd, message, sizeof(message));
        printf("Message sent: %s\n", message);
    }

    return NULL;
}

int main(int argc, char ** argv)
{
    if (argc != 2) return 1;

    int client_process_id = getpid();
    int server_process_id = atoi(argv[1]);
    int signal_manager_fd = signal_manager_init();
    int file_monitor = epoll_create1(0);

    add_file_descriptor(file_monitor, signal_manager_fd);
    kill(server_process_id, SIGUSR1);

    int client_pipe_fds[2];
    make_pipes(client_pipe_fds, client_process_id, CLIENT);

    pthread_t input_thread;
    pthread_create(&input_thread, NULL, write_thread, (void *) &client_pipe_fds[0]);

    FILE * read_file = fdopen(client_pipe_fds[1], "r");

    while (1)
    {

        struct epoll_event ready_events[MAX_EVENTS];
        int n = epoll_wait(file_monitor, ready_events, MAX_EVENTS, -1);

        printf("message has beeen received!\n");

        for (int i = 0; i < n; i++)
        {
            if (ready_events[i].data.fd == signal_manager_fd)
            {
                struct signalfd_siginfo info;
                read(signal_manager_fd, &info, sizeof(info));
                continue;
            }

            char buffer[100];
            if (fgets(buffer, sizeof(buffer), read_file) == NULL)
            {
                continue;
            }
        }
    }

    return 0;
}