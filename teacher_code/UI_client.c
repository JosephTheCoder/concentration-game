#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "board_library.h"
#include "UI_library.h"
#include "server.h"
#include "UI_client.h"

#define BUFFER_SIZE 128

int sock_fd = 0;
int dim , n = 0;

int terminate = 0;

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

    int play_x, play_y;
    char str_play[3];
    int color[3];

    int winner;

    int play_origin;

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
        printf("buffer recebido no read plays: %s\n",buffer);

        if (code == 3)
        {
            sscanf(buffer, "3 %d %d %d %s %d %d %d", &winner, &play_x, &play_y, str_play, &color[0], &color[1], &color[2]);
            paint_card(play_x, play_y, color[0], color[1], color[2]);
            write_card(play_x, play_y, str_play, text_color[0], text_color[1], text_color[2]); //receive text color from server
            
            printf("The winner is the Player %d!\n", winner);
            break;
        }

        // turn card down
        else if (code == -1)
        {
            sscanf(buffer, "-1 %d %d %d", &play_origin, &play_x, &play_y);
            paint_card(play_x, play_y, background_color[0], background_color[1], background_color[2]);
        }

        // turn card up
        else
        {
            sscanf(buffer, "1 %d %d %d %s %d %d %d", &play_origin, &play_x, &play_y, str_play, &color[0], &color[1], &color[2]);

            printf("Paint cell %d %d with the color %d %d %d\n", play_x, play_y, color[0], color[1], color[2]);

            paint_card(play_x, play_y, color[0], color[1], color[2]);
            write_card(play_x, play_y, str_play, text_color[0], text_color[1], text_color[2]); //receive text color from server
        }
    }
}

void read_board()
{
    int play_x, play_y;
    char str_play[3];
    int color[3];
    char buffer[BUFFER_SIZE];
    int n;

    while (strcmp(buffer, "board_sent") != 0)
    {
        memset(buffer, 0, BUFFER_SIZE);
        n = read(sock_fd, buffer, sizeof(buffer));
        // buffer[sizeof(buffer)]='\0';

        if (n == -1)
        {
            perror("error reading cell state");
            exit(-1);
        }

        else if (strcmp(buffer, "board_sent") != 0)
        {
            sscanf(buffer, "%s %d %d %d %d %d", str_play, &color[0], &color[1], &color[2], &play_x, &play_y);

            printf("buffer: %s\n", buffer);
            paint_card(play_x, play_y, color[0], color[1], color[2]);
            write_card(play_x, play_y, str_play, text_color[0], text_color[1], text_color[2]);
        }
    }
}

void *read_sdl_events()
{
    int done = 0;
    SDL_Event event;
    char buffer[BUFFER_SIZE];

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
                printf("Im leaving the game!\n");
                write_payload(buffer, sock_fd);
                done = SDL_TRUE;
                terminate = 1;
                break;
            }

            case SDL_MOUSEBUTTONDOWN:
            {
                int board_x, board_y;
                get_board_card(event.button.x, event.button.y, &board_x, &board_y);
                if(board_x<dim && board_y<dim)
                {
                    // send play to server
                    memset(buffer, 0, BUFFER_SIZE);
                    sprintf(buffer, "%d %d\n", board_x, board_y);
                    printf("Sending play: %s\n", buffer);
                    write_payload(buffer, sock_fd);
                    
                }
            }
            }
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    pthread_t thread_ID_read_sdl_events;
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
    printf("player color: [%d,%d,%d]\n", my_color[0], my_color[1], my_color[2]);

    create_board_window(300, 300, dim);

    read_board();

    printf("Received all the board info\n");

    /* Start game (copy from memory-single) */
    pthread_create(&thread_ID_read_sdl_events, NULL, read_sdl_events, NULL);

    read_plays();

    printf("fim\n");
    close_board_windows();

    close(sock_fd);
}