#define PORT "3000"

#include "board_library.h"
#include "server.h"

#include <time.h>

pthread_mutex_t **lock;
player_t *players_list_head = NULL;

/**************************************************************************************************/

player_t *find_fd_list(int fd)
{
    player_t *current = players_list_head;

    while (current->next != NULL)
    {
        if (current->fd != fd)
            current = current->next;
    }

    return current;
}

/**************************************************************************************************/

void update_cell_color(int x, int y, int r, int g, int b)
{
    int i = linear_conv(x, y);

    board[i].color[0] = r;
    board[i].color[1] = g;
    board[i].color[2] = b;
}
/**************************************************************************************************/

int *random_color()
{
    static int color[3];
    int i;

    for (i = 0; i < 3; i++)
        color[i] = rand() % 255;

    return color;
}

/**************************************************************************************************/

void *read_second_play(void *sock_fd)
{
    int fd = *((int *)sock_fd);
    int x = 0, y = 0;
    char buffer[128] = {'\0'};
    play_response resp;

    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);

        //add timer
        //if timer ends -> pthread_exit(flag) = -1 (tempo acabou)

        read(fd, buffer, sizeof(buffer));
        //buffer[strlen(buffer)] = '\0';

        sscanf(buffer, "%d %d", &x, &y);
        printf("Buffer 2nd play: %s\n", buffer);
        resp = board_play(x, y);
    }

    pthread_exit(&resp.code);
}

/***********************************************************************************************************/

void *send_play_to_all(void *buffer) //arg = string com posição jogada
{
    player_t *current = players_list_head;
    char *arr = (char *)buffer;

    while (current->next != NULL)
    {
        write(current->fd, arr, strlen(arr));
        current = current->next;
    }
    pthread_exit(NULL);
}

/*****************************************************************************************+*****/
void *read_first_play(void *sock_fd)
{
    // inserir mutexes nesta thread para evitar que dois clientes carreguem na mesma caixa na board

    int fd = *((int *)sock_fd);
    printf("fd: %d\n", fd);

    int x = 0, y = 0, code = 0;
    char buffer[128] = {'\0'};
    player_t *current = players_list_head;
    pthread_t thread_ID_secondPlay, thread_ID_sendPlays;
    play_response resp;

    current = find_fd_list(fd);

    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);
        read(fd, buffer, sizeof(buffer));
        //buffer[sizeof(buffer)] = '\0';

        if (strcmp(buffer, "exiting") == 0)
        {
            //remove player from the list
            printf("Player %d exited!", current->number);
            break;
        }

        sscanf(buffer, "%d %d", &x, &y);

        printf("Buffer 1st play: %s\n", buffer);

        pthread_mutex_lock(&lock[x][y]);
        resp = board_play(x, y);

        switch (resp.code)
        {
        case 0:
            /* chose filled position - Does nothing */
            memset(buffer, 0, BUFFER_SIZE); //erase buffer before inserting data
            sprintf(buffer, "%d", resp.code);
            write(fd, buffer, sizeof(buffer));
            break;
        case 1:
            /* first play */
            memset(buffer, 0, BUFFER_SIZE);
            sprintf(buffer, "%d %d %d %s %d %d %d %d %d %d", resp.code, resp.play1[0], resp.play1[1], resp.str_play1, current->color[0], current->color[1], current->color[2], 200, 200, 200);

            update_cell_color(resp.play1[0], resp.play1[1], current->color[0], current->color[1], current->color[2]);

            // construção buffer
            pthread_create(&thread_ID_sendPlays, NULL, send_play_to_all, (void *)buffer);

            //creates thread for second play, (read with timer)
            pthread_create(&thread_ID_secondPlay, NULL, read_second_play, (void *)&fd);

            //pthread join, receives code as return
            pthread_join(thread_ID_secondPlay, (void *)&code);

            switch (code)
            {
            case 0:
                /* chose filled position - Does nothing */
                // construção buffer a dizer "nononono", virar 1a carta para baixo
                memset(buffer, 0, BUFFER_SIZE);
                sprintf(buffer, "0 %d %d %d %d %d", resp.play1[0], resp.play1[1], 255, 255, 255);
                update_cell_color(resp.play1[0], resp.play1[1], 107, 200, 100);

                pthread_create(&thread_ID_sendPlays, NULL, send_play_to_all, (void *)buffer);
                break;

            case 2:
                pthread_create(&thread_ID_sendPlays, NULL, send_play_to_all, (void *)buffer);

            case -2:
                // buffer a virar a carta para cima
                //REVER CORES DO TEXTO E DA CELULA
                memset(buffer, 0, BUFFER_SIZE);
                sprintf(buffer, "%d %d %d %s %d %d %d %d %d %d", resp.code, resp.play2[0], resp.play2[1], resp.str_play2, current->color[0], current->color[1], current->color[2], 200, 200, 200);
                update_cell_color(resp.play2[0], resp.play2[1], current->color[0], current->color[1], current->color[2]);

                pthread_create(&thread_ID_sendPlays, NULL, send_play_to_all, (void *)buffer);
                pthread_join(thread_ID_sendPlays, NULL);
                sleep(2);

                // buffer a virar as cartas para baixo
                memset(buffer, 0, BUFFER_SIZE);

                sprintf(buffer, "%d %d %d %s %d %d %d %d %d %d", resp.code, resp.play1[0], resp.play1[1], resp.str_play1, 255, 255, 255, 255, 255, 255);
                update_cell_color(resp.play1[0], resp.play1[1], 107, 200, 100);

                pthread_create(&thread_ID_sendPlays, NULL, send_play_to_all, (void *)buffer);

                // buffer a virar as cartas para baixo
                memset(buffer, 0, BUFFER_SIZE);
                sprintf(buffer, "%d %d %d %s %d %d %d %d %d %d", resp.code, resp.play2[0], resp.play2[1], resp.str_play2, 255, 255, 255, 255, 255, 255);
                update_cell_color(resp.play2[0], resp.play2[1], 107, 200, 100);

                pthread_create(&thread_ID_sendPlays, NULL, send_play_to_all, (void *)buffer);

            case 3:
                //envia a todos a info para virar a carta e que o jogador x ganhou
                memset(buffer, 0, BUFFER_SIZE);
                sprintf(buffer, "%d %d %d %d %s %d %d %d %d %d %d", resp.code, current->number, resp.play2[0], resp.play2[1], resp.str_play2, 255, 255, 255, 255, 0, 0);
                update_cell_color(resp.play2[0], resp.play2[1], current->color[0], current->color[1], current->color[2]);

                pthread_create(&thread_ID_sendPlays, NULL, send_play_to_all, (void *)buffer);
            }
            break;
        }
    }
    pthread_exit(NULL);
}

int translate_i_to_x(int i, int dim_board)
{
    int x = i % dim_board;
    return x;
}

int translate_i_to_y(int i, int dim_board)
{
    int y = i / dim_board;
    return y;
}

void send_state_board(int fd, int dim_board)
{
    int i = 0;
    char str[12];
    char color[11] = {'\0'};

    for (i = 0; i < dim * dim; i++)
    {
        memset(buffer, 0, BUFFER_SIZE);

        strcpy(buffer, board[i].v);
        strcat(buffer, " ");
        sprintf(color, "%d", board[i].color[0]);
        strcat(buffer, color);
        strcat(buffer, " ");
        sprintf(color, "%d", board[i].color[1]);
        strcat(buffer, color);
        strcat(buffer, " ");
        sprintf(color, "%d", board[i].color[2]);
        strcat(buffer, color);
        strcat(buffer, " ");

        // coordenadas x e y da celula da board
        sprintf(str, "%d", translate_i_to_x(i, dim_board));
        strcat(buffer, str);
        strcat(buffer, " ");
        sprintf(str, "%d", translate_i_to_y(i, dim_board));
        strcat(buffer, str);

        printf("Sending cell: %s\n", buffer);

        write(fd, buffer, sizeof(buffer));
    }

    memset(buffer, 0, BUFFER_SIZE);
    sprintf(buffer, "%s", "board_sent");
    write(fd, buffer, sizeof(buffer));
}

// Change this to insert on the head of the list
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

int main(int argc, char *argv[])
{
    struct sockaddr_in local_addr, client_addr;

    int i = 0;
    int nr_players = 0;
    socklen_t size_addr;
    int new_fd;

    int send_state = 0;

    char buffer[BUFFER_SIZE];

    int *color;

    pthread_t thread_ID;

    srand(time(NULL));

    // dim par
    // dim < 26 e > 1

    // ---- Read dim argument and init board ----
    if (argc != 2 || sscanf(argv[1], "%d", &dim) == 0 || dim > 26 || dim < 1 || dim % 2 != 0)
    {
        printf("Please provide a correct dimension argument.\n");
        exit(1);
    }

    init_board(dim);

    lock = (pthread_mutex_t **)malloc(dim * sizeof(pthread_mutex_t *));
    for (i = 0; i < dim; i++)
    {
        lock[i] = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    }

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
        size_addr = sizeof(client_addr);
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
        sprintf(buffer, "%d/%d/%d/%d", dim, color[0], color[1], color[2]);
        write(new_fd, buffer, sizeof(buffer));

        // only start the game when there is more than 1 player
        if (nr_players == 2)
        {
            send_state = 1;
        }

        if (send_state == 1)
        {
            player_t *current = players_list_head;

            while (current != NULL)
            {
                send_state_board(current->fd, dim);
                current = current->next;
            }
        }

        pthread_create(&thread_ID, NULL, read_first_play, (void *)&new_fd);
    }

    pthread_mutex_destroy(*lock);

    close(sock_fd);
    return 0;
}
