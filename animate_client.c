
#include <signal.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <unistd.h>
#include "network.h"

enum client_states
{
    waiting,
    login,
    connected,
    disconnect,
    running
};

char name[100];

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
        char buffer[100];
        struct epoll_event ready_event;
        int n = epoll_wait(monitor, &ready_event, 1, -1);

        if (n == -1)
        {
            continue;
        }

        if (ready_event.events & EPOLLIN)
        {
            switch (state)
            {
                case login:
                    
                    fgets(buffer, 100, read_file);
                    if (atoi(buffer) > 0)
                    {
                        name[strcspn(name, "\n")] = '\0';
                        printf("Welcome %s. Your balance is %s\n", name, buffer);
                        fflush(stdout);

                        state = running;
                    }
                    else if (atoi(buffer)  == -2)
                    {
                        printf("Reject UNAUTHORISED\n");
                        fflush(stdout);
                    }
                    else
                    {
                        printf("Reject BALANCE\n");
                        fflush(stdout);
                    }
                break;

                case running:
                    fgets(buffer, 100, read_file);
                    printf("%s", buffer);
                    fflush(stdout);
                break;
            }
        }
    }
}

int main(int argc, char ** argv)
{
    char buffer[100];

    if (argc != 2) return 1;
    int * file_descriptors = (int *) malloc( sizeof(int) * 2 );
    int server_pid = atoi(argv[1]);
    int client_pid = getpid();

    pthread_t input_thread;

    struct sigaction action;
    action.sa_sigaction = &signal_handler;
    action.sa_flags = SA_SIGINFO;
    sigemptyset(&action.sa_mask);

    sigaction(SIGUSR1, &action, NULL);
    sigaction(SIGUSR2, &action, NULL);

    pthread_create(&input_thread, NULL, &manage_inputs, (void *) &( file_descriptors[1] ) );
    pthread_detach(input_thread);

    kill(server_pid, SIGUSR1);

    while (1)
    {
        switch (state)
        {
            case connected:
                make_pipes(file_descriptors, client_pid, CLIENT);
                state = login;
            break;

            case login:
                while (1)
                {
                    fgets(buffer, 100, stdin);

                    char message[100];
                    strcpy(message, buffer);

                    char * instruction = strtok(buffer, " ");
                    char * argument = strtok(NULL, " ");

                    if (strcmp(instruction, "Login") == 0)
                    {
                        strcpy(name, argument);
                        write(file_descriptors[0], message, strlen(message));
                    }
                    else if (strcmp(instruction, "Disconnect") == 0)
                    {
                        state = disconnect;
                    }
                }
            break;

            case running:
                fgets(buffer, 100, stdin);

                if (strcmp(instruction, "Disconnect"))
                {
                    state = disconnect
                }

                write(file_descriptors[0], buffer, strlen(buffer));
            break;

            case waiting:
            break;

            case disconnect:
                write(file_descriptors[0], "Disconnect\n", strlen("Disconnect\n"));

                close(file_descriptors[0]);
                close(file_descriptors[1]);

                char c2s[64];
                snprintf(c2s, sizeof(c2s), "./FIFO_C2S_%d", client_pid);

                char s2c[64];
                snprintf(s2c, sizeof(s2c), "./FIFO_S2C_%d", client_pid);

                unlink(c2s);
                unlink(s2c);
            break;
        }
    }
    free(file_descriptors);

    return 0;
}