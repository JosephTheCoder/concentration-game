
#include "UI_client.h"

/***********************************************************************************
 * write_payload()
 * 
 * Envia as jogadas ao servidor
 * 
 * *********************************************************************************/
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
 * read_plays()
 * 
 * Lê as jogadas enviadas pelo servidor e desenha as cores nas casa correspondentes
 * Detecta se é para virar uma casa para baixo ou para cima
 * Detecta se um dos jogadores ganhou a partida
 * 
 * *********************************************************************************/
void read_plays()
{   char *p;
    int n=0, i=0, cnt=0;
    int won = 0;
    int code = 0;
    int winner;
    int play_origin;
    int play_x, play_y;
    char buffer1[BUFFER_SIZE] = {'\0'};
    char buffer[BUFFER_SIZE] = {'\0'};
    char resto[BUFFER_SIZE] = {'\0'};
    char str_play[3];
    int color[3];

    // Receive response from server
    while (!done)
    {
        cnt = 0;
        n = 0;
        
        memset(buffer1, 0, BUFFER_SIZE);
        n = read(sock_fd, buffer1, BUFFER_SIZE);
<<<<<<< HEAD
        buffer1[strlen(buffer1)] = '\0';
        
        // printf("strlen(buffer1)=%ld\n", strlen(buffer1));
=======
        buffer[strlen(buffer)]='\0';
        if(n == -1)
        {
            perror("error reading play response");
            exit(-1);
        }
>>>>>>> f9e4890bd01061d029305d4286d5b0fa79b6fef5

        for (i = 0; i < strlen(buffer1) - 1; i++)
        {
            if (buffer1[i] == '\n')
            {
                buffer1[i] = ',';
                cnt += 1;
            }
        }

        printf("Received play response with %d bytes: %s\n", n, buffer1);
        printf("cnt: %d\n", cnt);
        if (cnt == 0)
        {
            sscanf(buffer1, "%[^\n]s\n", buffer);
            printf("buffer: %s\n", buffer);
        }
        else if (cnt > 0)
        {
            printf("%d\n", sscanf(buffer1, "%[^,]s,", buffer));     
            for (p = buffer1; p < buffer1+strlen(buffer1); p++)
            {
                if (*p == ',')
                {
                    p = p + 1;
                    break;
                }
            }
            strcpy(resto, p);
            printf("buffer: %s\n", buffer);
            printf("resto: %s\n", resto);
        }

        // Enquanto ainda ha mensagens para ler no buffer1
        while (cnt > -1)
        {
            sscanf(buffer, "%d", &code);
            printf("buffer recebido no read plays: %s\n", buffer);
            printf("code: %d\n", code);

            char *pch;

            // Winner or Looser
            if (code == 3) // se algum jogador ganha
            {
                pch = strtok(buffer, " ");
                while (pch != NULL)
                {
                    pch = strtok(NULL, " ");
                    sscanf(pch, "%d", &winner);

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

            // turn card down
            else if (code == -1) // se é para virar uma casa para baixo
            {
                sscanf(buffer, "-1 %d %d %d", &play_origin, &play_x, &play_y);
                paint_card(play_x, play_y, background_color[0], background_color[1], background_color[2]);
            }

            // turn card up
            else // se é para virar uma carta para cima
            {
                sscanf(buffer, "1 %d %d %d %s %d %d %d", &play_origin, &play_x, &play_y, str_play, &color[0], &color[1], &color[2]);

                printf("Paint cell %d %d with the color %d %d %d\n", play_x, play_y, color[0], color[1], color[2]);

                paint_card(play_x, play_y, color[0], color[1], color[2]);
                write_card(play_x, play_y, str_play, text_color[0], text_color[1], text_color[2]); //receive text color from server
            }
            // menos uma mensagem para ler do buffer
            cnt--;

            if(cnt==0) // se ja só exite uma mensagem
                sscanf(resto,"%[^\n]s\n", buffer);
            else if(cnt>0) // se existe mais do que uma mensagem para ler
             {
                 sscanf(resto, "%[^,]s,", buffer);
                 for (p = resto; p < resto+strlen(resto); p++)
                {
                    if (*p == ',')
                    {
                        p = p + 1;
                        break;
                    }
                }
                strcpy(resto, p);
                printf("buffer: %s\n", buffer);
                printf("resto: %s\n", resto);
             }   
        
        }
    }
}

/***********************************************************************************
 * read_board()
 * 
 * Recebe os dados da board quando começa a jogar.
 * 
 * *********************************************************************************/

void read_board()
{
    int n;
    int play_x, play_y;
    char str_play[3];
    int color[3];
    char buffer[BUFFER_SIZE];
 

    // recebe todos os dados da board
    while (strcmp(buffer, "board_sent") != 0)
    {
        memset(buffer, 0, BUFFER_SIZE);
        n = read(sock_fd, buffer, sizeof(buffer));
        buffer[sizeof(buffer)]='\0';

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

/***********************************************************************************
 * read_sdl_events()
 * 
 * Lê do teclado:
 *  _ se fechou a janela, acaba a sessao do jogador;
 *  _ se jogou uma jogada ( cliquou numa casa ).
 * 
 * *********************************************************************************/

void *read_sdl_events()
{
    
    SDL_Event event;
    char buffer[BUFFER_SIZE];
    int board_x=0, board_y=0;

    while (!done)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT: // carrega no botao "X" da tabela de jogo
            {
                // send message to server saying we're about to quit
                memset(buffer, 0, BUFFER_SIZE);
                strcpy(buffer, "exiting");
                printf("Im leaving the game!\n");
                write_payload(buffer, sock_fd);   
                close_board_windows();       
                done = 1;
            }
            case SDL_MOUSEBUTTONDOWN:
            {   if(done==0)
                {
                     get_board_card(event.button.x, event.button.y, &board_x, &board_y);
                    if (board_x < dim && board_y < dim)
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
    }
    pthread_exit(NULL);
}

/***********************************************************************************
 * read_sdl_events()
 * 
 * Lê do teclado:
 *  _ se fechou a janela, acaba a sessao do jogador;
 *  _ se jogou uma jogada ( cliquou numa casa ).
 * 
 * *********************************************************************************/

int main(int argc, char *argv[])
{
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    pthread_t thread_ID_read_sdl_events;
    n = 0;
    dim = 0;
    done=0;

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
    buffer[strlen(buffer)]='\0';
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

    pthread_create(&thread_ID_read_sdl_events, NULL, read_sdl_events, NULL);
    read_plays();
    pthread_join(thread_ID_read_sdl_events, NULL);
    printf("fim\n");

    close(sock_fd);
    return 0;
}