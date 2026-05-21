#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int threadpool_size;
int server_pid;

int main(int argc, char** argv, char** envp) 
{
    server_pid = getpid();
    threadpool_size = atoi(argv[1]);
    printf("Server PID: %d\n", server_pid);
    
    return 0;
}
