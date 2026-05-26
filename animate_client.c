#include <pthread.h>
#include <signal.h>
#include "network.h"
#include <stdatomic.h>
#include "debug.h"

enum client_states
{
    start,
    login,
    running,
    disconnect
};

atomic_int state = start;
atomic_int alive = 1;

static char name[100];
pthread_mutex_t name_lock = PTHREAD_MUTEX_INITIALIZER;

void signal_handler(int signum, siginfo_t* info, void * context)
{
    (void) context;
    (void) info;

    if (signum == SIGUSR2) state = login;
    if (signum == SIGUSR1) alive = 0;
}

void * send_to_server(void * args)
{
    FILE ** files = (FILE **) args;

    char buffer[100];
    while (alive)
    {
        switch (state)
        {
            case login:
                #if DEBUG
                    fprintf(stderr, "(loggin client) out\n");
                #endif

                fgets(buffer, 100, stdin);

                char instruction[50];
                char argument[50];
                sscanf(buffer, "%s %s", instruction, argument);

                if (strcmp(instruction, "Login") == 0)
                {
                    fprintf(files[1], "%s", buffer);
                    fflush (files[1]);

                    pthread_mutex_lock(&name_lock);
                    strcpy(name, argument);
                    pthread_mutex_unlock(&name_lock);
                }
                else if (strcmp(instruction, "Disconnect") == 0)
                {
                    fprintf(files[1], "Disconnect");
                    fflush(files[1]);
                    state = disconnect;
                }

                while (state == login);
            break;



            case running:
                #if DEBUG
                    fprintf(stderr, "(running client) out\n");
                #endif

                ssize_t bytes_read = read(STDIN_FILENO, buffer, 99);

                if (!alive) return NULL;

                if (bytes_read > 0)
                {
                    buffer[bytes_read] = '\0';  /* null-terminate manually */

                    if ((strcmp(buffer, "Disconnect\n") == 0) || (strcmp(buffer, "Disconnect") == 0))
                    {
                        state = disconnect;
                    }

                    fprintf(files[1], "%s", buffer);
                    fflush(files[1]);
                }
            break;

            case disconnect:
                #if DEBUG
                    fprintf(stderr, "(disconnect client) out\n");
                #endif
                alive = 0;
            break;
        }
    }

    return NULL;
}

void * recieve_from_server(void * args)
{
    FILE ** files = (FILE **) args;

    char buffer[100];
    while (alive)
    {
        switch (state)
        {
            case login:
                #if DEBUG
                    fprintf(stderr, "(loggin client) in\n");
                #endif

                ssize_t bytes = read(fileno(files[0]), buffer, 99);
                if (bytes <= 0) 
                    break;
                
                buffer[bytes] = '\0';
                printf("%s", buffer);
                fflush(stdout);

                int status = atoi(buffer);

                if (status >= 1)
                {
                    pthread_mutex_lock(&name_lock);
                    printf("Welcome %s. Your balance is %s", name, buffer);
                    fflush(stdout);
                    pthread_mutex_unlock(&name_lock);
                    state = running;
                }
                else if (status == -1)
                {
                    printf("Reject BALANCE\n");
                    fflush(stdout);

                    state = disconnect;
                }
                else 
                {
                    printf("Reject UNAUTHORISED\n");
                    fflush(stdout);

                    state = disconnect;
                }
                memset(buffer, 0, 100);
            break;

            case running:
                #if DEBUG
                    fprintf(stderr, "(running client) in\n");
                #endif

                char * running_status_in = fgets(buffer, 100, files[0]);

                if (running_status_in == NULL)
                {
                    alive = 0;
                    break;
                }

                if (running_status_in != NULL)
                {
                    printf("%s", buffer);
                    fflush(stdout);
                }
            break;
        }
    }

    return NULL;
}

int main (int argc, char ** argv)
{
    if (argc != 2)
    {
        return 1;
    }

    int client_pid = getpid();
    int server_pid = atoi(argv[1]);

    if (server_pid == 0)
    {
        return 1;
    }

    struct sigaction action;
    action.sa_sigaction = &signal_handler;
    action.sa_flags = SA_SIGINFO;
    sigemptyset(&action.sa_mask);
    sigaction(SIGUSR2, &action, NULL);
    sigaction(SIGUSR1, &action, NULL);

    kill(server_pid, SIGUSR1);

    while (1)
    {
        if (state != start) break;
    }

    FILE ** files = (FILE **) malloc(sizeof(FILE *) * 2);
    open_pipes_client(files, client_pid);

    pthread_t sending_thread;
    pthread_t receiving_thread;

    pthread_create(&sending_thread, NULL, send_to_server, (void *) files);
    pthread_create(&receiving_thread, NULL, recieve_from_server, (void *) files);

    pthread_join(sending_thread, NULL);
    pthread_join(receiving_thread, NULL);

    pthread_mutex_destroy(&name_lock);
    close_pipes(files, client_pid);
    free(files);
    return 0;
}

