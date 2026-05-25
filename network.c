

#include "network.h"

#include <errno.h>
#include <string.h>
#include <stdio.h>

/*************************************************************************
    Signal Processing + Pipes
-------------------------------------------------------------------------*/

void make_pipes( int * fds, int process_id, int type)
{
    char c2s[64];
    snprintf(c2s, sizeof(c2s), "/tmp/FIFO_C2S_%d", process_id);

    char s2c[64];
    snprintf(s2c, sizeof(s2c), "/tmp/FIFO_S2C_%d", process_id);

    if (mkfifo(c2s, 0666) == -1)
    {
        if (errno != EEXIST)
        perror("mkfifo c2s failed");
    }

    if (mkfifo(s2c, 0666) == -1)
    {
        if (errno != EEXIST)
        perror("mkfifo s2c failed");
    }

    if (type == SERVER)
    {
        fds[0] = open(c2s, O_RDONLY);
        fds[1] = open(s2c, O_WRONLY);
    }
    if (type == CLIENT)
    {
        fds[0] = open(c2s, O_WRONLY);
        fds[1] = open(s2c, O_RDONLY);
    } 
}

/*************************************************************************
    Checking if a user exists 
-------------------------------------------------------------------------*/

bool check_user_login(char *username, char *file_path, int *return_balance)
{
    FILE *users_list = fopen(file_path, "r");
    if (users_list == NULL)
    {
        return false;
    }

    char * clean_username = strtok(username, "\n");
    char line[64];

    while (fgets(line, sizeof(line), users_list) != NULL)
    {
        char *name = strtok(line, " \n");
        *return_balance = atoi(strtok(NULL, " \n"));

        if (strcmp(name, clean_username) == 0)
        {
            return (*return_balance > 0);
        }
    }

    fclose(users_list);
    return false;
}

/*************************************************************************
    ePolling Functions 
-------------------------------------------------------------------------*/

void add_file_descriptor (int epoll_fd, int file_descriptor)
{
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = file_descriptor;

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, file_descriptor, &event);
}

/*************************************************************************
    Networking Example Test
-------------------------------------------------------------------------*/
/*
int main(int argc, char ** argv)
{
    if (strcmp(argv[1], "server") == 0)
    {
        int server_pid = getpid();
        printf("Server PID: %d", &server_pid);
        
        int signal_manager_fd = signal_manager_init();
        int file_monitor = epoll_create1(0);

        add_file_descriptor(file_monitor, signal_manager_fd);

        int number_events = 1;

        while (1)
        {
            struct epoll_event ready_events[number_events];
            epoll_wait(file_monitor, ready_events, number_events, -1);

            for (int i = 0; i < number_events; i ++)
            {
                if (ready_events[i].data.fd == signal_manager_fd)
                {
                    if (ready_events[i].events & EPOLLIN)
                    {
                        struct signalfd_siginfo info;
                        read(signal_manager_fd, &info, sizeof(info));

                        kill(info.ssi_pid, SIGUSR2);
                        printf("Signal has been received!");

                        int fds[2];
                        make_pipes(fds, info.ssi_pid ,SERVER);

                        add_file_descriptor(file_monitor, fds[0]);
                        break;
                    }
                }
            }
        }
    }
    else if (strcmp(argv[1], "client") == 0)
    {
        int server_pid = argv[2];


    }
    else
    {
        printf("Invalid Service Type!");
        return 1;
    }

    return 0;
}

*/

/*************************************************************************
    User Login Test
-------------------------------------------------------------------------*/
/* 
int main(int argc, char ** argv)
{
    int balance;
    if (check_user_login(argv[1], "./user.txt", &balance))
    {
        printf("THE USER EXISTS AND HAS BALANCE\n");
    }
    else
    {
        printf("THE USER DOES NOT EXIST OR DOES NOT HAVE BALANCE!\n");
    }
}
*/

/*************************************************************************
    Signal Processing + Pipes Testing
-------------------------------------------------------------------------*/

/*
int main(int argc, char ** argv)
{
    if (strcmp(argv[1], "server") == 0)
    {
        if (argc < 2) return 1;

        int server_process_id      = getpid();
        printf("Server Process ID: %d\n", server_process_id);
        int signal_file_descriptor = signal_manager_init();

        struct pollfd poll_struct =
        {
            .fd = signal_file_descriptor,
            .events = POLLIN
        };

        poll(&poll_struct, 1, -1);

        struct signalfd_siginfo info;
        read(signal_file_descriptor, &info, sizeof(info));

        kill(info.ssi_pid, SIGUSR2);

        int server_pipe_fds[2] = {0, 0};
        make_pipes(server_pipe_fds, info.ssi_pid, SERVER);

        printf("The Data Has Been Received!\n");
    }
    else if (strcmp(argv[1], "client") == 0)
    {
        if (argc < 3) return 1;

        int client_process_id = getpid();
        int server_process_id = atoi(argv[2]);
        int signal_file_descriptor = signal_manager_init();

        struct pollfd poll_struct = 
        {
            .fd = signal_file_descriptor,
            .events = POLLIN
        };

        kill(server_process_id, SIGUSR1);
        poll(&poll_struct, 1, -1);

        int client_pipe_fds[2] = {0, 0};
        make_pipes(client_pipe_fds, client_process_id, CLIENT);

        printf("The Data Has Been Received!\n");
    }
    else
    {
        printf("Invalid Service Type!");
        return 1;
    }

    return 0;
}
*/

/*
int main()
{
    int i = fork();

    if (i == 0)
    {
        int fds[2];
        make_pipes(fds, getpid(), CLIENT);

        char message[50];
        read(fds[1], message, sizeof(message));

        printf("Message received: %s\n", message);
    }
    else
    {
        int fds[2];
        make_pipes(fds, i, SERVER);

        write(fds[1], "Hello World!", sizeof("Hello World!"));
        printf("Message sent: Hello World!\n");
    }
}
*/