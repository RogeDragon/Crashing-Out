#include "network.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>

void * write_thread(void * arg)
{
    int * write_fd = (int *) arg;

    while (1)
    {
        char message[50];

        if (fgets(message, sizeof(message), stdin) == NULL)
        {
            break;
        }

        size_t length = strlen(message);
        if (length > 0)
        {
            write(*write_fd, message, length);
        }
    }

    return NULL;
}

void * read_thread(void * arg)
{
    FILE * read_file = (FILE *) arg;

    while (1)
    {
        char message[50];

        if (fgets(message, sizeof(message), read_file) == NULL)
        {
            break;
        }

        printf("%s", message);
    }

    fclose(read_file);
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
    pthread_t input_thread;
    pthread_t read_thread_id;

    while (1)
    {
        struct epoll_event ready_events[2];
        int n = epoll_wait(file_monitor, ready_events, 2, -1);

        for (int i = 0; i < n; i++)
        {
            if (ready_events[i].data.fd == signal_manager_fd)
            {
                struct signalfd_siginfo sig_info;
                if (read(signal_manager_fd, &sig_info, sizeof(sig_info)) == -1)
                {
                    perror("read");
                    continue;
                }

                make_pipes(client_pipe_fds, client_process_id, CLIENT);

                FILE * read_file = fdopen(client_pipe_fds[1], "r");
                if (read_file == NULL)
                {
                    perror("fdopen");
                    return 1;
                }

                pthread_create(&input_thread, NULL, write_thread, (void *) &client_pipe_fds[0]);
                pthread_detach(input_thread);
                pthread_create(&read_thread_id, NULL, read_thread, (void *) read_file);
                pthread_detach(read_thread_id);
                break;
            }
        }
    }

    return 0;
}