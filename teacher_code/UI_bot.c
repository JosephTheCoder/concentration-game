
#include "UI_bot.h"

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

void read_plays()
{
    int code = 0;
    char buffer[BUFFER_SIZE] = {'\0'};

    int play[2];
    char str_play[3];
    int text_color[3];
    int color[3];

    int position_index;
    int play_origin;

    memory_place *position_in_memory = NULL;
    playable_place *random_place = NULL;

    int winner;

    int n;

    // Receive response from server
    while (!terminate)
    {
        n = 0;
        memset(buffer, 0, BUFFER_SIZE);
        n = read(sock_fd, buffer, BUFFER_SIZE);

        printf("Received play response with %d bytes: %s\n", n, buffer);

        if (n == -1)
        {
            perror("error reading play response");
            exit(-1);
        }

        sscanf(buffer, "%d", &code);
        printf("buffer recebido no read plays: %s\n", buffer);

        if (code == 3)
        {
            sscanf(buffer, "3 %d %d %d %s %d %d %d", &winner, &play[0], &play[1], str_play, &color[0], &color[1], &color[2]);
            paint_card(play[0], play[1], color[0], color[1], color[2]);
            write_card(play[0], play[1], str_play, text_color[0], text_color[1], text_color[2]); //receive text color from server

            printf("The winner is the Player %d!\n", winner);
            break;
        }

        else if (code == -1)
        {
            sscanf(buffer, "-1 %d %d %d %s", &play_origin, &play[0], &play[1], str_play);
            paint_card(play[0], play[1], background_color[0], background_color[1], background_color[2]);
            save_playable_position(play);

            sleep(2);

            if (play_origin == player_number)
            {
                save_in_memory(str_play, play);
                bot_play_number = FIRST_PLAY;
            }
        }

        else
        {
            sscanf(buffer, "%d %d %d %s %d %d %d %d %d %d", &code, &play[0], &play[1], str_play, &color[0], &color[1], &color[2], &text_color[0], &text_color[1], &text_color[2]);

            printf("Paint cell %d %d with the color %d %d %d\n", play[0], play[1], color[0], color[1], color[2]);

            paint_card(play[0], play[1], color[0], color[1], color[2]);
            write_card(play[0], play[1], str_play, text_color[0], text_color[1], text_color[2]); //receive text color from server
            
            sleep(2);
            
            remove_playable_position(play);

            if (bot_play_number == FIRST_PLAY)
                bot_play_number = SECOND_PLAY;
            
            // SEGMENTATION FAULT
            // If the bot has already played the first card, makes second play
            if (bot_play_number == SECOND_PLAY)
            {                
                // check if the bot has seen a card like the one played
                position_in_memory = find_relative_in_memory(str_play);

                if (position_in_memory != NULL)
                {
                    memset(buffer, 0, BUFFER_SIZE);
                    sprintf(buffer, "%d %d", position_in_memory->position[0], position_in_memory->position[1]);
                    printf("Sending play: %s\n", buffer);
                    write_payload(buffer, sock_fd);
                }

                // if he has never seen one, play a random card from the playable list
                else
                {
                    position_index = rand() % nr_playable_positions;

                    random_place = get_playable_position(position_index);

                    memset(buffer, 0, BUFFER_SIZE);
                    sprintf(buffer, "%d %d", random_place->position[0], random_place->position[1]);

                    printf("Sending play: %s\n", buffer);

                    write_payload(buffer, sock_fd);
                }
            }
        }
    }
}

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
        n = read(sock_fd, buffer, sizeof(buffer));
        //buffer[sizeof(buffer)]='\0';

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

            else
            {
                printf("Adding playable position\n");
                save_playable_position(play);
            }
        }
    }
}

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
                done = SDL_TRUE;
                terminate = 1;
                break;
            }
            }
        }
    }

    pthread_exit(NULL);
}

void *generate_first_play(void *arg)
{
    int dim = *((int *)arg);
    int position_index;
    char buffer[BUFFER_SIZE] = {'\0'};

    playable_place *random_place;

    while (!terminate)
    {
        if (bot_play_number == FIRST_PLAY)
        {
            position_index = rand() % nr_playable_positions;

            random_place = get_playable_position(position_index);

            memset(buffer, 0, BUFFER_SIZE);
            sprintf(buffer, "%d %d", random_place->position[0], random_place->position[1]);

            printf("Sending play: %s\n", buffer);

            write_payload(buffer, sock_fd);
            bot_play_number = SECOND_PLAY;
        }
    }

    pthread_exit(NULL);
}

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

void save_in_memory(char *letters, int *position)
{
    // entra na cabeça, apaga a ultima
    memory_place *new_place = (memory_place *)malloc(sizeof(memory_place));

    new_place->next = bot_memory;
    new_place->position[0] = position[0];
    new_place->position[1] = position[1];

    bot_memory = new_place;
    nr_memory_positions++;

    // if max memory has been reached, deletes oldest position
    if (nr_memory_positions > MAX_POSITIONS_IN_MEMORY)
    {
        memory_place *second_last = bot_memory;

        while (second_last->next->next != NULL)
            second_last = second_last->next;

        free(second_last->next);
        second_last->next = NULL;

        nr_memory_positions--;
    }
}

memory_place *find_relative_in_memory(char *letters)
{
    memory_place *current = bot_memory;

    while (current->next != NULL)
    {
        if (current->v[0] == letters[0] && current->v[1] == letters[1] && current->v[2] == letters[2])
            return current;
        else
            current = current->next;
    }

    return NULL;
}

int main(int argc, char *argv[])
{
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

    pthread_create(&thread_ID_read_sdl_events, NULL, read_sdl_events, NULL); // change this cause function only reads SDL_QUIT

    pthread_create(&thread_ID_generate_plays, NULL, generate_first_play, (void *)&dim);

    read_plays();

    printf("fim\n");
    close_board_windows();

    close(sock_fd);
}