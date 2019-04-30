#define PORT "3000"

#include "board_library.h"
#include "UI_library.h"
#include "server.h"

#include <time.h>

int players_fd[MAX_PLAYERS];

char rand_color()
{
    int x = 0;
    char color[11] = {'\0'};

    //pode haver jogadores com cores muito parecidas
    x = rand() % 255;
    strcat(color, x);
    strcat(color, "/");
    x = rand() % 255;
    strcat(color, x);
    strcat(color, "/");
    x = rand() % 255;
    strcat(color, x);
    return color;
}

void *send_new_player_info(void *x)
{
    int i = *(int *)x;
    while (1)
    {
        if (write(players_fd[i], &players[i], sizeof(players[i])) > 0)
            printf("Sent player number\n");

        write(players_fd[i], &dim, sizeof(dim));
        stcpy(color, rand_color());
        write(players_fd[i], color, strlen(color));

        clear_board(&b);

        while (b.winner == ' ')
        {
            write(players_fd[i], &b, sizeof(b));
            play_remote(&b, players[i], players_fd[i]);
            write(players_fd[i], &b, sizeof(b));
            printf("Sent board\n");
        }
    }
    pthread_exit(NULL);
}

void main(int argc, char *argv[])
{
    struct sockaddr_in local_addr, client_addr;

    int dim = 0, i = 0;
    int nb_players = 0;
    char color[11] = {'\0'};
    int size_addr = 0;

    pthread_t thread_ID;
    srand(time(NULL));

    if (argc != 2 || sscanf(argv[1], "%d", &dim) == 0)
    {
        printf("Please provide a correct dimension argument.\n");
        exit(1);
    }

    init_board(dim); //mudar parametros de board

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1)
    {
        perror("socket: ");
        exit(-1);
    }

    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(CONCENTRATION_GAME_PORT);
    local_addr.sin_addr.s_addr = INADDR_ANY;

    n = bind(sock_fd, (struct sockaddr *)&local_addr, sizeof(local_addr));
    if (n == -1)
    {
        perror("bind");
        exit(-1);
    }

    if (listen(sock_fd, 10) == -1)
        exit(1);

    while (1)
    {
        // Waiting for players
        for (i = 0; i < 2; i++)
        {
            size_addr = sizeof(client_addr);
            players_fd[i] = accept(sock_fd, (struct sockaddr *)&client_addr, &size_addr);

            nb_players++;

            pthread_create(&thread_ID, NULL, send_new_player_info, (int *)&i);
        }

        // thread a enviar state of board
        else
        {

            srcpty(buffer, board.v);
            strcat(buffer, color);

            for (i = 0; i < (dim ^ 2); i++)
            {

                write(newfd, buffer, strlen(buffer));
            }
            pthread_create(&thread_ID, NULL, thread_fcn, (int *)newfd);
        }
    }

    freeaddrinfo(res);
    close(newfd);
    close(fd);
}
