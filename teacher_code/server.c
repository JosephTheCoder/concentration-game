#define PORT "3000"

#include "board_library.h"
#include "UI_library.h"
#include "server.h"

#include <time.h>

player_t *players_list_head = NULL;

int *random_color()
{
    static int color[3];
    int i;

    for (i = 0; i < 3; i++)
        color[i] = rand() % 255;

    return color;
}

// void *accept_new_players(void *sock_fd)
// {
//     int player = 2;

//     while (1)
//     {
//         size_addr = sizeof(client_addr);
//         players_fd[player] = accept(sock_fd, (struct sockaddr *)&client_addr, &size_addr);

//         write(players_fd[player], &dim, sizeof(dim));
//         stcpy(color, rand_color());
//         write(players_fd[player], color, strlen(color));

//         send_state_board(players_fd[player]);

//         player++;
//     }

//     pthread_exit(NULL);
// }

// void *comunication_server_players(void *x)
// {
//     int i = *(int *)x;
//     while (1)
//     {
//         send_state_board(players_fd[i]);
//     }
//     pthread_exit(NULL);
// }

void send_state_board(int fd)
{
    int i;
    char str[12];
    char color[11] = {'\0'};
    int sent_cell = 0;

    for (i = 0; i < dim * dim; i++)
    {
        if (board[i].color[0] != 107 && board[1].color[2] != 200 && board[3].color[3] != 100)
        {
            memset(buffer, 0, BUFFER_SIZE);

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

            sprintf(str, "%d", i);
            strcat(buffer, str);

            printf("buffer: %s\n", buffer);
            write(fd, buffer, sizeof(buffer));

            sent_cell = 1;
        }
    }

    // in case of an empty board
    if (sent_cell == 0)
    {
        strcpy(buffer, "empty_board");
        write(fd, buffer, sizeof(buffer));
    }
}

void push_to_list(player_t *head, int *color, int fd)
{
    player_t *current = head;
    int number = 1;

    while (current->next != NULL)
    {
        current = current->next;
        number++;
    }

    /* now we can add a new variable */
    current->next = malloc(sizeof(player_t));

    current->next->fd = fd;
    current->next->number = number;

    current->next->color[0] = color[0];
    current->next->color[1] = color[1];
    current->next->color[2] = color[2];

    current->next->next = NULL;
}

/* https://www.learn-c.org/en/Linked_lists
int remove_from_list(player_t *head, int number)
{
    int i = 0;
    int player_number = -1;
    player_t *current = *head;
    player_t *temp_node = NULL;

    if (number == 1)
    {
        player_t *node = head;
        *head = *head->next;
        free(node);
    }

    for (i = 1; i < number - 1; i++)
    {
        if (current->next == NULL)
        {
            return -1;
        }
        current = current->next;
    }

    temp_node = current->next;
    player_number = temp_node->number;
    current->next = temp_node->next;

    close(temp_node->fd);
    free(temp_node);

    return player_number;
} */

void main(int argc, char *argv[])
{
    struct sockaddr_in local_addr, client_addr;

    int i = 0;
    int nr_players = 0;
    int size_addr = sizeof(client_addr);
    int new_fd;

    int send_state = 0;

    char buffer[BUFFER_SIZE];

    int *color;

    pthread_t thread_ID;

    srand(time(NULL));

    // ---- Read dim argument and init board ----
    if (argc != 2 || sscanf(argv[1], "%d", &dim) == 0)
    {
        printf("Please provide a correct dimension argument.\n");
        exit(1);
    }
    init_board(dim);

    // ---- Setup TCP server ----
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1)
    {
        perror("socket");
        exit(-1);
    }

    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(CONCENTRATION_GAME_PORT);
    local_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock_fd, (struct sockaddr *)&local_addr, sizeof(local_addr)) == -1)
    {
        perror("bind");
        exit(-1);
    }

    if (listen(sock_fd, 10) == -1)
    {
        perror("listen");
        exit(1);
    }

    // ---- Main loop ----
    while (1)
    {
        // Waiting for players
        new_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &size_addr);
        if (new_fd == -1)
        {
            perror("accept");
            exit(-1);
        }

        nr_players++;

        printf("Player %d connected!\n", nr_players);

        color = random_color();

        if (nr_players == 1) // in the case of the 1st player we also have to allocate the list!
        {
            players_list_head = malloc(sizeof(player_t));
            if (players_list_head == NULL)
                exit(1);

            players_list_head->number = nr_players;
            players_list_head->fd = new_fd;
            players_list_head->color[0] = color[0];
            players_list_head->color[1] = color[1];
            players_list_head->color[2] = color[2];
            players_list_head->next = NULL;
        }

        else // 2nd player case -> push to existing list
        {
            push_to_list(players_list_head, color, new_fd);
        }

        memset(buffer, 0, BUFFER_SIZE); //erase buffer before inserting data
        sprintf(buffer, "%d", dim);
        write(new_fd, buffer, sizeof(buffer));

        memset(buffer, 0, BUFFER_SIZE);
        sprintf(buffer, "%d/%d/%d", color[0], color[1], color[2]);
        write(new_fd, buffer, sizeof(buffer));

        if (nr_players == 2)
            send_state = 1;


        if (send_state == 1)
        {
            player_t *current = players_list_head;

            while (current != NULL)
            {
                send_state_board(current->fd);
                current = current->next;
            }
        }

        //thread to listen the players plays
        // read x y
        //board_play(board_x, board_y);

        // basta uma thread por jogador(em principio)
        // if (nr_players = 2) // se for o 2º jogador então cria a thread do 1º(que nao podia jogar sozinho) e do 2º
        // {
        //     for (i = 0; i < 2; i++)
        //         //envia o state of the board para todos os que estavam a espera
        //         pthread_create(&thread_ID, NULL, accept_new_players, i);
        // }

        // else
        // {
        //     pthread_create(&thread_ID, NULL, comunication_server_players, nr_players);
        // }
    }

    close(sock_fd);
}
