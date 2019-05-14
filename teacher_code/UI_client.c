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

void *read_play_response(void *buffer) //arg = string com posição jogada
{
    // Receive response from server
    memset(buffer, 0, BUFFER_SIZE);
    if (read(sock_fd, buffer, BUFFER_SIZE) == -1)
    {
        perror("error reading play response");
        exit(-1);
    }

    sscanf(buffer, "%d", &code);

    if (code == 3)
    {
        //acabou
    }
    else if (code == 0)
    {
        // does nothing
        sscanf(buffer, "0/%d/%d", &resp.play[0], &resp.play[1]);
        paint_card(resp.play[0], resp.play[1], 255, 255, 255);
    }
    else
    {
        sscanf(buffer, "%d/%d/%d/%s/%d/%d/%d/%d/%d/%d", &code, &resp.play[0], &resp.play[1], resp.str_play, &color[0], &color[1], &color[2], &text_color[0], &text_color[1], &text_color[2]);
        paint_card(resp.play[0], resp.play[1], color[0], color[1], color[2]);
        write_card(resp.play[0], resp.play[1], resp.str_play, 200, 200, 200); //receive text color from server
    }
}

int main(int argc, char *argv[])
{

    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    server_response resp;
    int code;

    int color[3];
    int my_color[3];
    int text_color[3];

    SDL_Event event;
    int done = 0;
    int dim = 0;

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

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
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
    if (read(sock_fd, buffer, BUFFER_SIZE) == -1)
    {
        perror("error reading dimension of board");
        exit(-1);
    }

    sscanf(buffer, "%d/%d/%d/%d", &dim, &my_color[0], &my_color[1], &my_color[2]);

    printf("board dimension: %d\n", dim);
    create_board_window(300, 300, dim);

    resp.str_play = (char *)malloc(dim * sizeof(char));

    printf("player color: [%d,%d,%d]\n", my_color[0], my_color[1], my_color[2]);

    // WROOOONG
    for (int i = 0; i < dim * dim; i++)
    {
        memset(buffer, 0, BUFFER_SIZE);
        if (read(sock_fd, buffer, BUFFER_SIZE) == -1)
        {
            perror("error reading cell state");
            exit(-1);
        }

        if (strcmp(buffer, "sent_board") == 0)
        {
            printf("Received all the board info\n");
            break;
        }

        else
        {
            sscanf(buffer, "%d/%d/%d/%d", &dim, &my_color[0], &my_color[1], &my_color[2]);
            // Player connected when the game is already running

            // UPDATE BOARD -----------------
        }

        printf("%s\n", buffer);
    }

    /* Start game (copy from memory-single) */
    while (!done)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
            {
                done = SDL_TRUE;
                // send message to server saying we're about to quit
                memset(buffer, 0, BUFFER_SIZE);
                sprintf(buffer, "exiting");
                write(sock_fd, buffer, sizeof(buffer));
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
            {
                // pthread_create(send_play)

                int board_x, board_y;
                get_board_card(event.button.x, event.button.y, &board_x, &board_y);

                printf("click (%d %d) -> (%d %d)\n", event.button.x, event.button.y, board_x, board_y);

                // send play to server
                memset(buffer, 0, BUFFER_SIZE);
                sprintf(buffer, "%d/%d", board_x, board_y);
                write(sock_fd, buffer, sizeof(buffer));
            }
            }
        }
    }

    printf("fim\n");
    close_board_windows();

    close(sock_fd);
}