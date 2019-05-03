#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "board_library.h"
#include "UI_library.h"
#include "server.h"

#define BUFFER_SIZE 128

int main(int argc, char *argv[])
{
    int fd;
    ssize_t n;
    socklen_t addrlen;
    struct sockaddr_in server_addr;
    char buffer[BUF FER_SIZE];

    int color[3];

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

    /* Read board dimension info */
    if (read(sock_fd, buffer, BUFFER_SIZE) == -1)
    {
        perror("error reading dimension of board");
        exit(-1);
    }

    sscanf(buffer, "%d", &dim);

    memset(buffer, 0, BUFFER_SIZE);
    if (read(sock_fd, buffer, BUFFER_SIZE) == -1)
    {
        perror("error reading player color");
        exit(-1);
    }

    int r, g, b;
    sscanf(buffer, "%d/%d/%d", &r, &g, &b);

    printf("board dimension: %d\n", dim);
    printf("player color: [%d,%d,%d]\n", r, g, b);

    color[0] = r;
    color[1] = g;
    color[2] = b;

    for (int i = 0; i < dim * dim; i++)
    {
        memset(buffer, 0, BUFFER_SIZE);
        if (read(sock_fd, buffer, BUFFER_SIZE) == -1)
        {
            perror("error reading cell state");
            exit(-1);
        }

        if(strcmp(buffer, "empty_board") == 0) 
        {
            printf("Board is still empty\n");
            break;
        }
        else
        {
            

        }

        printf("Cell %d info received!\n", i);
        printf("%s\n", buffer);
    }

    /* Init board window */
    //create_board_window(300, 300, dim);
    //init_board(dim);

    // /* Start game (copy from memory-single) */
    // while (!done)
    // {
    //     while (SDL_PollEvent(&event))
    //     {
    //         switch (event.type)
    //         {
    //         case SDL_QUIT:
    //         {
    //             done = SDL_TRUE;
    //             break;
    //         }
    //         case SDL_MOUSEBUTTONDOWN:
    //         {
    //             int board_x, board_y;
    //             get_board_card(event.button.x, event.button.y, &board_x, &board_y);

    //             printf("click (%d %d) -> (%d %d)\n", event.button.x, event.button.y, board_x, board_y);
    //             play_response resp = board_play(board_x, board_y);
    //             switch (resp.code)
    //             {
    //             case 1:
    //                 paint_card(resp.play1[0], resp.play1[1], 7, 200, 100);
    //                 write_card(resp.play1[0], resp.play1[1], resp.str_play1, 200, 200, 200);
    //                 break;
    //             case 3:
    //                 done = 1;
    //             case 2:
    //                 paint_card(resp.play1[0], resp.play1[1], 107, 200, 100);
    //                 write_card(resp.play1[0], resp.play1[1], resp.str_play1, 0, 0, 0);
    //                 paint_card(resp.play2[0], resp.play2[1], 107, 200, 100);
    //                 write_card(resp.play2[0], resp.play2[1], resp.str_play2, 0, 0, 0);
    //                 break;
    //             case -2:
    //                 paint_card(resp.play1[0], resp.play1[1], 107, 200, 100);
    //                 write_card(resp.play1[0], resp.play1[1], resp.str_play1, 255, 0, 0);
    //                 paint_card(resp.play2[0], resp.play2[1], 107, 200, 100);
    //                 write_card(resp.play2[0], resp.play2[1], resp.str_play2, 255, 0, 0);
    //                 sleep(2);
    //                 paint_card(resp.play1[0], resp.play1[1], 255, 255, 255);
    //                 paint_card(resp.play2[0], resp.play2[1], 255, 255, 255);
    //                 break;
    //             }
    //         }
    //         }
    //     }
    // }
    // printf("fim\n");
    // close_board_windows();

    close(fd);
}