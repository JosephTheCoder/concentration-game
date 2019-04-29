#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ws2tcpip.h>

#define PORT "3000"
#define BUFFER_SIZE 128

int main(int argc, char *argv[])
{
    int fd;
    ssize_t n;
    socklen_t addrlen;
    struct addrinfo hints, *res;
    struct sockaddr_in addr;
    char buffer[BUFFER_SIZE];

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

    /* Init TCP */
    memset(&hints, 0, sizeof hints);
    hints.a_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flag = AI_NUMERICSERV;

    if (getaddrinfo("", PORT, &hints, &res) != 0)
        exit(1);

    if (connect(fd, res->ai_addr, res->ai_addrlen) == -1)
        exit(1);

    /* Read board dimension info */
    n = read(fd, buffer, BUFFER_SIZE);
    if (n == -1)
        exit(1);

    sscanf(buffer, "%d", &dim);

    /* Init board window */
    create_board_window(300, 300, dim);
    init_board(dim);

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
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
            {
                int board_x, board_y;
                get_board_card(event.button.x, event.button.y, &board_x, &board_y);

                printf("click (%d %d) -> (%d %d)\n", event.button.x, event.button.y, board_x, board_y);
                play_response resp = board_play(board_x, board_y);
                switch (resp.code)
                {
                case 1:
                    paint_card(resp.play1[0], resp.play1[1], 7, 200, 100);
                    write_card(resp.play1[0], resp.play1[1], resp.str_play1, 200, 200, 200);
                    break;
                case 3:
                    done = 1;
                case 2:
                    paint_card(resp.play1[0], resp.play1[1], 107, 200, 100);
                    write_card(resp.play1[0], resp.play1[1], resp.str_play1, 0, 0, 0);
                    paint_card(resp.play2[0], resp.play2[1], 107, 200, 100);
                    write_card(resp.play2[0], resp.play2[1], resp.str_play2, 0, 0, 0);
                    break;
                case -2:
                    paint_card(resp.play1[0], resp.play1[1], 107, 200, 100);
                    write_card(resp.play1[0], resp.play1[1], resp.str_play1, 255, 0, 0);
                    paint_card(resp.play2[0], resp.play2[1], 107, 200, 100);
                    write_card(resp.play2[0], resp.play2[1], resp.str_play2, 255, 0, 0);
                    sleep(2);
                    paint_card(resp.play1[0], resp.play1[1], 255, 255, 255);
                    paint_card(resp.play2[0], resp.play2[1], 255, 255, 255);
                    break;
                }
            }
            }
        }
    }
    printf("fim\n");
    close_board_windows();

    freeaddring(res);
    close(fd);
}