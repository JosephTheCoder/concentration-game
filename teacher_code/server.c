#define PORT "3000"

#include "board_library.h"
#include "server.h"

#include <time.h>

pthread_mutex_t **lock;
play_response resp[100];
player_t *players_list_head = NULL;

/**************************************************************************************************/

player_t *find_fd_list(int fd)
{
    player_t *current = players_list_head;

    while (current->fd != fd)
        current = current->next;

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

int write_payload(char *payload, int fd)
{
    int written = 0;
    int n;

    while (written < strlen(payload))
    {
        if ((n = write(fd, payload + written, strlen(payload) - written)) < 0)
        {
            return -1;
        }

        written += n;
    }

    return written;
}

/**************************************************************************************************/

void *read_second_play(void *sock_fd)
{
    
    int fd = *((int *)sock_fd);
    int x = 0, y = 0, rv=0;
    char buffer[BUFFER_SIZE] = {'\0'};

    fd_set set;
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    FD_ZERO(&set); /* clear the set */
    FD_SET(fd, &set);
    rv=select(fd+1, &set, NULL,NULL, &timeout); 
    if(rv<0) 
    {
        perror("select\n"); /* an error accured */
    }else if(rv==0)
    {
        resp[fd] = board_play(x, y, fd, 1);
        printf("timeout\n"); /* a timeout occured */
    }else{
       read(fd, buffer, sizeof(buffer));

        if (strcmp(buffer, "exiting") == 0)
        {
            resp[fd].code = 4;
            pthread_exit(NULL);
        }

        sscanf(buffer, "%d %d\n", &x, &y);
        printf("Buffer 2nd play: %s\n", buffer);
        pthread_mutex_lock(&lock[x][y]);
        resp[fd] = board_play(x, y, fd, 0); //terceiro argumento diz que está tudo OK

        printf("code play 2: %d\n", resp[fd].code);

        if (resp[fd].code == 0)
            pthread_mutex_unlock(&lock[x][y]);
    }
     printf("saí do select\n");
    
    //pthread_exit((void*)&resp);
    pthread_exit(NULL);
}

/***********************************************************************************************************/

void *send_play_to_all(void *buffer) //arg = string com posição jogada
{
    player_t *current = players_list_head;
    char *payload = (char *)buffer;

    int n;

    while (current != NULL)
    {
        n = write_payload(payload, current->fd);
        printf("Sent payload with %d bytes to player %d: %s\n", n, current->number, payload);
        current = current->next;
    }

    pthread_exit(NULL);
}

/***********************************************************************************************************/

void broadcast_up(int origin_player, int x, int y, char *str, int *color)
{
    pthread_t thread_ID_sendPlays;
    char buffer[BUFFER_SIZE] = {'\0'};

    sprintf(buffer, "1 %d %d %d %s %d %d %d", origin_player, x, y, str, color[0], color[1], color[2]);

    // construção buffer
    pthread_create(&thread_ID_sendPlays, NULL, send_play_to_all, (void *)buffer);
    pthread_join(thread_ID_sendPlays, NULL);
}

/***********************************************************************************************************/

void broadcast_down(int origin_player, int x, int y, char *str)
{
    pthread_t thread_ID_sendPlays;
    char buffer[BUFFER_SIZE] = {'\0'};

    // memset(buffer, 0, BUFFER_SIZE);
    sprintf(buffer, "-1 %d %d %d %s", origin_player, x, y, str);

    // construção buffer
    pthread_create(&thread_ID_sendPlays, NULL, send_play_to_all, (void *)buffer);
    pthread_join(thread_ID_sendPlays, NULL);
}

/***********************************************************************************************************/

void broadcast_winner(int player, int x, int y, char *str, int *color)
{
    pthread_t thread_ID_sendPlays;
    char buffer[BUFFER_SIZE] = {'\0'};

    sprintf(buffer, "3 %d %d %d %s %d %d %d", player, x, y, str, color[0], color[1], color[2]);

    // construção buffer
    pthread_create(&thread_ID_sendPlays, NULL, send_play_to_all, (void *)buffer);
}

/*****************************************************************************************+*****/
void *read_first_play(void *sock_fd)
{
    int fd = *((int *)sock_fd);
    printf("fd: %d\n", fd);

    int x = 0, y = 0;
    char buffer[BUFFER_SIZE] = {'\0'};

    player_t *current = players_list_head;
    pthread_t thread_ID_secondPlay;

    current = find_fd_list(fd);
    int terminate = 0;
    
    char str[3];

    while (!terminate)
    {
        memset(buffer, 0, BUFFER_SIZE);
        read(fd, buffer, sizeof(buffer));
        //buffer[sizeof(buffer)] = '\0';

        if (strcmp(buffer, "exiting") == 0)
        {
            //remove player from the list
            remove_from_list(players_list_head, current->number);
            printf("Player %d exited!", current->number);
            break;
        }

        sscanf(buffer, "%d %d\n", &x, &y);
        printf("Buffer 1st play: %s\n", buffer);
        
        pthread_mutex_lock(&lock[x][y]);
        resp[fd] = board_play(x, y, fd, 0); // o terceiro argumento diz que nao é para fazer cancel da jogada
        printf("resp[fd] first play: %d\n", resp[fd].code);
        switch (resp[fd].code)
        {
        case 0:
            /* chose filled position - Does nothing */
            pthread_mutex_unlock(&lock[x][y]);
            break;
        case 1:
            /* first play */
            resp[fd].code=0;
            update_cell_color(resp[fd].play1[0], resp[fd].play1[1], current->color[0], current->color[1], current->color[2]);
            broadcast_up(current->number, resp[fd].play1[0], resp[fd].play1[1], resp[fd].str_play1, current->color);
            pthread_mutex_unlock(&lock[resp[fd].play1[0]][resp[fd].play1[1]]);
            
            str[0] = resp[fd].str_play1[0];
            str[1] = resp[fd].str_play1[1];
            str[2] = resp[fd].str_play1[2];

            //creates thread for second play, (read with timer)
            pthread_create(&thread_ID_secondPlay, NULL, read_second_play, (void *)&fd);

            //pthread join, receives code as return
            pthread_join(thread_ID_secondPlay, NULL);

            //printf("code play 2: %d\n", code);
            printf("code play 2_first thread: %d\n", resp[fd].code);
            switch (resp[fd].code)
            {
            case 0:

                update_cell_color(x, y, 255, 255, 255);

                resp[fd] = board_play(x, y, fd, 1); // nao envia jogada, apenas faz cancel da jogada e recomeca a play 1
                broadcast_down(current->number, x, y, str);
                break;

            case 2:

                update_cell_color(resp[fd].play2[0], resp[fd].play2[1], current->color[0], current->color[1], current->color[2]);
                broadcast_up(current->number, resp[fd].play2[0], resp[fd].play2[1], resp[fd].str_play2, current->color);
                pthread_mutex_unlock(&lock[resp[fd].play2[0]][resp[fd].play2[1]]);
                break;

            case -2:

                update_cell_color(resp[fd].play2[0], resp[fd].play2[1], current->color[0], current->color[1], current->color[2]);
                broadcast_up(current->number, resp[fd].play2[0], resp[fd].play2[1], resp[fd].str_play2, current->color);

                sleep(2);

                update_cell_color(resp[fd].play2[0], resp[fd].play2[1], 255, 255, 255);
                // adiccionar str ao broadcast down
                broadcast_down(current->number, resp[fd].play2[0], resp[fd].play2[1], resp[fd].str_play2);
                pthread_mutex_unlock(&lock[resp[fd].play2[0]][resp[fd].play2[1]]);

                update_cell_color(resp[fd].play1[0], resp[fd].play1[1], 255, 255, 255);
                broadcast_down(current->number, resp[fd].play1[0], resp[fd].play1[1], str);
                break;

            case 3:
                //envia a todos a info para virar a carta e que o jogador x ganhou
                update_cell_color(resp[fd].play2[0], resp[fd].play2[1], current->color[0], current->color[1], current->color[2]);
                broadcast_winner(current->number, resp[fd].play2[0], resp[fd].play2[1], resp[fd].str_play2, current->color);
                pthread_mutex_unlock(&lock[resp[fd].play2[0]][resp[fd].play2[1]]);
                break;

            case 4:
                //remove player from the list
                //remove_from_list(players_list_head, current->number);
                printf("Player %d exited!", current->number);
                terminate = 1;
                break;
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
    char buffer[BUFFER_SIZE] = {'\0'};

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
    write_payload(buffer, fd);
}

// Change this to insert on the head of the list
void push_to_list(player_t *head, int *color, int fd, int player_number)
{
    player_t *current = head;

    while (current->next != NULL)
        current = current->next;

    /* now we can add a new variable */
    current->next = (player_t *)malloc(sizeof(player_t));

    current->next->fd = fd;
    current->next->number = player_number;

    current->next->color[0] = color[0];
    current->next->color[1] = color[1];
    current->next->color[2] = color[2];

    current->next->next = NULL;
}

int remove_from_list(player_t *head, int player_number)
{
    if (!head)
        return -1;

    player_t *temp = head;
    ;
    player_t *prev = NULL;

    while (temp->number != player_number && temp->next != NULL)
    {
        prev = temp;
        temp = temp->next;
    }

    if (temp->number == player_number)
    {
        if (prev)
        {
            prev->next = temp->next;
        }
        else
        {
            head = temp->next;
        }
        free(temp);
        return player_number;
    }

    return -1;
}

int main(int argc, char *argv[])
{
    struct sockaddr_in local_addr, client_addr;

    int i = 0;
    int nr_players = 0;
    socklen_t size_addr;
    int new_fd;

    int send_state = 0;

    char buffer[BUFFER_SIZE] = {'\0'};

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
        lock[i] = (pthread_mutex_t *)malloc(dim * sizeof(pthread_mutex_t));
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
            players_list_head = (player_t *)malloc(sizeof(player_t));
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
            push_to_list(players_list_head, color, new_fd, nr_players);
        }

        sprintf(buffer, "%d %d %d %d %d", nr_players, dim, color[0], color[1], color[2]);
        write_payload(buffer, new_fd);

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
