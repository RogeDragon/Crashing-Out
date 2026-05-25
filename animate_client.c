
#include <signal.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <unistd.h>
#include "network.h"

enum client_states
{
    waiting,
    connected,
    running
};

static volatile sig_atomic_t state = waiting;

void signal_handler(int signum, siginfo_t* info, void * context) 
{
    (void) context;
    (void) info;

    if (signum == SIGUSR2)
    {
        state = connected;
    }
}

void * manage_inputs( void * arg )
{
    int monitor = epoll_create1(0);
    int read_fd = *((int*) arg);
    FILE * read_file = fdopen(read_fd , "r");

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = read_fd;

    epoll_ctl(monitor, EPOLL_CTL_ADD, read_fd, &event);

    while (1)
    {
        struct epoll_event ready_event;
        int n = epoll_wait(monitor, &ready_event, 1, -1);

        if (n == -1)
        {
            continue;
        }

        if (ready_event.events & EPOLLIN)
        {
            char buffer[100];
            fgets(buffer, 100, read_file);
            printf("%s", buffer);
            fflush(stdout);
        }
    }
}

int file_descriptors[2];

int main(int argc, char ** argv)
{
    if (argc != 2) return 1;
    int server_pid = atoi(argv[1]);
    int client_pid = getpid();

    pthread_t input_thread;

    struct sigaction action;
    action.sa_sigaction = &signal_handler;
    action.sa_flags = SA_SIGINFO;
    sigemptyset(&action.sa_mask);

    sigaction(SIGUSR1, &action, NULL);
    sigaction(SIGUSR2, &action, NULL);

    kill(server_pid, SIGUSR1);

    while (1)
    {
        switch (state)
        {
            case connected:
                make_pipes(file_descriptors, client_pid, CLIENT);

                pthread_create(&input_thread, NULL, &manage_inputs, (void *) &( file_descriptors[1] ) );
                pthread_detach(input_thread);
                state = running;
            break;

            case running:
                char buffer[100];
                fgets(buffer, 100, stdin);
                write(file_descriptors[0], buffer, strlen(buffer));
                write(file_descriptors[0], "\n", 1);
            break;

            case waiting:
            break;
        }
    }

    return 0;
}