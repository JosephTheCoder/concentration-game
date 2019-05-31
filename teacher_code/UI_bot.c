
#include "UI_bot.h"


/***********************************************************************************
 * 
 * 
 * 
 * 
 * ********************************************************************************/
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


/***********************************************************************************
 * 
 * 
 * 
 * 
 * ********************************************************************************/
void read_plays()
{
    
    char buffer1[BUFFER_SIZE] = {'\0'};
    char buffer[BUFFER_SIZE] = {'\0'};
    char rest[BUFFER_SIZE] = {'\0'};
    char *p=NULL;
    int play[2]={0};
    char str_play[3]= {'\0'};;
    int color[3]={0};
    int play_origin=0;
    int code = 0;
    int winner=0;
    int won = 0;
    int cnt=0;
    int n=0, i=0;

    // Receive response from server
    while (!end)
    {
        
        memset(buffer1, 0, BUFFER_SIZE);
        n = read(sock_fd, buffer1, BUFFER_SIZE);
        buffer1[BUFFER_SIZE]='\0';
        if (n == -1)
        {
            perror("error reading play response");
            exit(-1);
        }
        // substitui "\n" no meio da string por virgulas para separar as diferentes mensagens
        for (i = 0; i < strlen(buffer1) - 1; i++)
        {
            if (buffer1[i] == '\n')
            {
                buffer1[i] = ',';
                cnt += 1;
            }
        }
        // cnt é o numero de mensagens recebidas
        printf("cnt: %d\n", cnt);
        if (cnt == 0)
        {
            sscanf(buffer1, "%[^\n]s\n", buffer);
            printf("buffer: %s\n", buffer);
        }
        else if (cnt > 0)
        {
            printf("%d\n", sscanf(buffer1, "%[^,]s,", buffer));     
            for (p = buffer1; p < buffer1 + strlen(buffer1); p++)
            {
                if (*p == ',')
                {
                    p = p + 1;
                    break;
                }
            }
            strcpy(rest, p);
            printf("buffer: %s\n", buffer);
            printf("resto: %s\n", rest);
        }

        // Enquanto ainda ha mensagens para ler no buffer1
        while(cnt>-1){

            sscanf(buffer, "%d", &code);
            printf("buffer recebido no read plays: %s\n", buffer);

            // receives WINNER signal
            if (code == 3)
            {
                if (code == 3)
                {
                    while (sscanf(buffer, "%d ", &winner) == 1)
                    {
                        if (winner == player_number)
                        {
                            printf("Player %d - You won! :)\n", player_number);
                            won = 1;
                        }
                    }

                    if (won == 0)
                    {
                        printf("Player %d - You lost! :(\n", player_number);
                    }

                    break;
                }
            }

            // receives signal to turn card DOWN
            else if (code == -1)
            {
                sscanf(buffer, "-1 %d %d %d %s", &play_origin, &play[0], &play[1], str_play);
                paint_card(play[0], play[1], background_color[0], background_color[1], background_color[2]);

                bot_status = SEND_PLAY;
            }

            // receives signal to turn card UP
            else
            {
                sscanf(buffer, "1 %d %d %d %s %d %d %d", &play_origin, &play[0], &play[1], str_play, &color[0], &color[1], &color[2]);
                printf("Paint cell %d %d with the color %d %d %d\n", play[0], play[1], color[0], color[1], color[2]);
                paint_card(play[0], play[1], color[0], color[1], color[2]);
                write_card(play[0], play[1], str_play, text_color[0], text_color[1], text_color[2]); //receive text color from server

                bot_status = SEND_PLAY;
            }

            // decrementa o numero de mensagens a ler ainda
            cnt--;
            if(cnt==0) // se ja só exite uma mensagem
                sscanf(rest,"%[^\n]s\n", buffer);
            else if(cnt>0) // se existe mais do que uma mensagem para ler
             {
                 sscanf(rest, "%[^,]s,", buffer);
                 for (p = rest; p < rest+strlen(rest); p++)
                {
                    if (*p == ',')
                    {
                        p = p + 1;
                        break;
                    }
                }
                strcpy(rest, p);
                printf("buffer: %s\n", buffer);
                printf("resto: %s\n", rest);
             }   
        
        }
    }
}


/***********************************************************************************
 * 
 * 
 * 
 * 
 * ********************************************************************************/
void read_board()
{
    int play[2];
    char str_play[3];
    int color[3];
    char buffer[BUFFER_SIZE];
    int n;

    while (strcmp(buffer, "board_sent") != 0)
    {
        memset(buffer, 0, BUFFER_SIZE);
        n = read(sock_fd, buffer, BUFFER_SIZE);
        buffer[BUFFER_SIZE]='\0';
        if (n == -1)
        {
            perror("error reading cell state");
            exit(-1);
        }

        else if (strcmp(buffer, "board_sent") != 0)
        {
            sscanf(buffer, "%s %d %d %d %d %d", str_play, &color[0], &color[1], &color[2], &play[0], &play[1]);

            paint_card(play[0], play[1], color[0], color[1], color[2]);

            if (color[0] != background_color[0] || color[1] != background_color[1] || color[2] != background_color[2])
            {
                write_card(play[0], play[1], str_play, 200, 200, 200);
            }
        }
    }
}


/***********************************************************************************
 * 
 * 
 * 
 * 
 * ********************************************************************************/
void *read_sdl_events()
{
    int done = 0;
    SDL_Event event;
    char buffer[BUFFER_SIZE] = {'\0'};

    while (!done)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
            {
                // send message to server saying we're about to quit
                memset(buffer, 0, BUFFER_SIZE);
                strcpy(buffer, "exiting");
                write_payload(buffer, sock_fd);
                close_board_windows();
                done = SDL_TRUE;
                end = 1;
                exit(0);
                break;
            }
            }
        }
    }

    pthread_exit(NULL);
}


/***********************************************************************************
 * 
 * 
 * 
 * 
 * ********************************************************************************/
void *generate_first_play(void *arg)
{
    int dim = *((int *)arg);
    char buffer[BUFFER_SIZE] = {'\0'};

    playable_place *random_place = (playable_place *)malloc(sizeof(playable_place));

    while (!end)
    {
        if (bot_status == SEND_PLAY)
        {
            random_place->position[0] = rand() % dim;
            random_place->position[1] = rand() % dim;

            // random_place = get_playable_position(position_index);

            memset(buffer, 0, BUFFER_SIZE);
            sprintf(buffer, "%d %d", random_place->position[0], random_place->position[1]);

            printf("Sending play: %s\n", buffer);

            write_payload(buffer, sock_fd);
            bot_status = WAITING_RESPONSE;
        }
    }
    pthread_exit(NULL);
}


/***********************************************************************************
 * 
 * 
 * 
 * 
 * ********************************************************************************/
void save_playable_position(int *new_position)
{
    playable_place *playable_pos = (playable_place *)malloc(sizeof(playable_place));

    playable_pos->next = playable_positions;
    playable_pos->position[0] = new_position[0];
    playable_pos->position[1] = new_position[1];

    // head = playable_pos;
    playable_positions = playable_pos;
    nr_playable_positions++;
}


/***********************************************************************************
 * 
 * 
 * 
 * 
 * ********************************************************************************/
void remove_playable_position(int *position)
{
    playable_place *temp = playable_positions;

    playable_place *prev = NULL;

    while (temp->position[0] != position[0] && temp->position[1] != position[1] && temp->next != NULL)
    {
        prev = temp;
        temp = temp->next;
    }

    if (temp->position[0] != position[0] && temp->position[1])
    {
        if (prev)
        {
            prev->next = temp->next;
        }
        else
        {
            playable_positions = temp->next;
        }
        free(temp);
        nr_playable_positions--;
    }
}

/***********************************************************************************
 * 
 * 
 * 
 * 
 * ********************************************************************************/

playable_place *get_playable_position(int index)
{
    int i = 0;
    playable_place *current = playable_positions;

    for (i = 0; i < index; i++)
    {
        printf("%d %d\n", current->position[0], current->position[1]);
        current = current->next;
    }

    return current;
}


/***********************************************************************************
 * 
 * 
 * 
 * 
 * ********************************************************************************/
int main(int argc, char *argv[])
{
    end=0;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    pthread_t thread_ID_read_sdl_events;
    pthread_t thread_ID_generate_plays;

    int n = 0;
    dim = 0;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(-1);
    }

    if (TTF_Init() == -1)
    {
        printf("TTF_Init: %s\n", TTF_GetError());
        exit(2);
    }

    if (argc < 2)
    {
        printf("second argument should be server address\n");
        exit(-1);
    }

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1)
    {
        perror("socket");
        exit(-1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(CONCENTRATION_GAME_PORT);
    inet_aton(argv[1], &server_addr.sin_addr);

    if (-1 == connect(sock_fd, (const struct sockaddr *)&server_addr, sizeof(server_addr)))
    {
        perror("connect");
        exit(-1);
    }

    /* Read board dimension and color info */
    n = read(sock_fd, buffer, BUFFER_SIZE);
    buffer[BUFFER_SIZE]='\0';

    if (n == -1)
    {
        perror("error reading dimension of board");
        exit(-1);
    }

    sscanf(buffer, "%d %d %d %d %d", &player_number, &dim, &my_color[0], &my_color[1], &my_color[2]);

    printf("player number %d\n", player_number);
    printf("board dimension: %d\n", dim);
    create_board_window(300, 300, dim);

    printf("player color: [%d,%d,%d]\n", my_color[0], my_color[1], my_color[2]);

    read_board();

    printf("Received all the board info\n");

    sleep(2);

    pthread_create(&thread_ID_read_sdl_events, NULL, read_sdl_events, NULL); // change this cause function only reads SDL_QUIT

    bot_status = SEND_PLAY;
    pthread_create(&thread_ID_generate_plays, NULL, generate_first_play, (void *)&dim);

    read_plays();

    printf("fim\n");
    close_board_windows();

    close(sock_fd);
}