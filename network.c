#include "network.h"

void create_pipes(int process_id)
{
    char server_to_client[50];
    char client_to_server[50];

    sprintf(server_to_client, "/tmp/FIFO_S2C_%d", process_id);
    sprintf(client_to_server, "/tmp/FIFO_C2S_%d", process_id);

    if (mkfifo(server_to_client, 0666) == -1)
        perror("mkfifo s2c failed");

    if (mkfifo(client_to_server, 0666) == -1)
        perror("mkfifo c2s failed");
}

void open_pipes_client(FILE **FILES, int process_id)
{
    char server_to_client[50];
    char client_to_server[50];

    sprintf(server_to_client, "FIFO_S2C_%d", process_id);
    sprintf(client_to_server, "FIFO_C2S_%d", process_id);

    FILES[1] = fopen(client_to_server, "w");
    FILES[0] = fopen(server_to_client, "r");
}

void open_pipes_server(FILE **FILES, int process_id, int * reading_fd)
{
    char server_to_client[50];
    char client_to_server[50];

    sprintf(server_to_client, "FIFO_S2C_%d", process_id);
    sprintf(client_to_server, "FIFO_C2S_%d", process_id);

    *reading_fd = open(client_to_server, O_RDONLY);
    FILES[0] = fopen(server_to_client,  "w");
    FILES[1] = fdopen(*reading_fd, "r");
}

int check_user_login(char *username, char *file_path)
{
    FILE *users_list = fopen(file_path, "r");

    char clean_username[100];
    strcpy(clean_username, username);
    clean_username[strcspn(clean_username, "\n")] = 0;

    char line[100];
    while (fgets(line, sizeof(line), users_list) != NULL)
    {
        char name[50];
        char balance[50];
        sscanf(line, "%s %s", name, balance);

        if (strcmp(clean_username, name) == 0)
        {
            int bal = atoi(balance);
            if (bal > 0)
            {
                return 1;
            }
            else if (bal < 0)
            {
                return -1;
            }
        }
    }

    printf("Finished Checking!");

    fclose(users_list);
    return 0; //does not exist
}

void close_and_unlink_pipes(FILE **FILES, int process_id)
{
    char server_to_client[50];
    char client_to_server[50];

    sprintf(server_to_client, "/tmp/FIFO_S2C_%d", process_id);
    sprintf(client_to_server, "/tmp/FIFO_C2S_%d", process_id);

    fclose(FILES[0]);
    fclose(FILES[1]);

    if (unlink(server_to_client) == -1)
        perror("unlink s2c failed");

    if (unlink(client_to_server) == -1)
        perror("unlink c2s failed");
}

void close_pipes(FILE **FILES, int process_id)
{
    char server_to_client[50];
    char client_to_server[50];

    sprintf(server_to_client, "/tmp/FIFO_S2C_%d", process_id);
    sprintf(client_to_server, "/tmp/FIFO_C2S_%d", process_id);

    fclose(FILES[0]);
    fclose(FILES[1]);
}