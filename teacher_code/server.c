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

void *  comunication_server_players(void *x)
{
    int i = *(int *)x;
    while (1)
    {
        send_state_board(players_fd[i]);
        
    }
    pthread_exit(NULL);
}

void send_state_board(int fd)
{
    int i;
    char color[11] = {'\0'};

    for (i = 0; i < (dim ^ 2); i++)
    {
        if (board[i].color[0] != 107 && board[1].color[2] != 200 && board[3].color[3] != 100)
        {
            strcpy(buffer, board[i].v);
            strcat(buffer, "/");
            sprintf(color, "%d", board[i].color[0]);
            strcat(buffer, color);
            strcat(buffer, "/");
            sprintf(color, "%d", board[i].color[1]);
            strcat(buffer, color);
            strcat(buffer, "/");
            sprintf(color, "%d", board[i].color[2]);
            strcat(buffer, color);
            strcat(buffer, "/");
            strcat(buffer, i);

            write(fd, buffer, strlen(buffer));
        }
    }
}

void main(int argc, char *argv[])
{
    struct sockaddr_in local_addr, client_addr;

    int i = 0;
    int nb_players = 0;
    int size_addr = 0;

    char color[11] = {'\0'};

    pthread_t thread_ID;
    srand(time(NULL));

    if (argc != 2 || sscanf(argv[1], "%d", &dim) == 0)
    {
        printf("Please provide a correct dimension argument.\n");
        exit(1);
    }

    init_board(dim); // por cores a preto [0,0,0]

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
        size_addr = sizeof(client_addr);
        players_fd[nb_players] = accept(sock_fd, (struct sockaddr *)&client_addr, &size_addr);
            if(players_fd[nb_players]==-1)
                exit(1);

        write(players_fd[nb_players], &dim, sizeof(dim));
        stcpy(color, rand_color());
        write(players_fd[nb_players], color, strlen(color));
        
        // basta uma thread por jogador(em principio)
        if(nb_players=2) // se for o 2º jogador então cria a thread do 1º(que nao podia jogar sozinho) e do 2º
        {
            for(i=0;i<2;i++)
                pthread_create(&thread_ID, NULL, accept_new_players, i);
        }else{
            pthread_create(&thread_ID, NULL, comunication_server_players, nb_players);
        }

        nb_players++;
    }

    freeaddrinfo(res);
    close(newfd);
    close(fd);
}
