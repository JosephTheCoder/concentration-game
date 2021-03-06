
#include "UI_bot.h"

/***********************************************************************************
 * write_payload()
 * 
 * Envia as jogadas ao servidor
 * 
 * *********************************************************************************/
int write_payload(char *payload, int fd)
{
    int written = 0;
    int n=0;

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
    int cnt=0;
    int n=0, i=0;

    // Receive response from server
    while (!end)
    {
        cnt=0;
        n=0;

        memset(buffer1, 0, BUFFER_SIZE);
        n = read(sock_fd, buffer1, BUFFER_SIZE);
         if (n == -1)
        {
            break;
        }
        buffer1[strlen(buffer1)]='\0';
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
        if (cnt == 0)
        {
            sscanf(buffer1, "%[^\n]s\n", buffer);
        }
        else if (cnt > 0)
        {
            sscanf(buffer1, "%[^,]s,", buffer);     
            for (p = buffer1; p < buffer1 + strlen(buffer1); p++)
            {
                if (*p == ',')
                {
                    p = p + 1;
                    break;
                }
            }
            strcpy(rest, p);

        }

        // Enquanto ainda ha mensagens para ler no buffer1
        while(cnt > -1){

            sscanf(buffer, "%d", &code);

            // Winner or Looser
            if (code == 3) // se algum jogador ganha
            {
                printf("Player %d - You WON! :)\n", player_number);
                reset_board(300, 300, dim);
                read_board();

                bot_status = SEND_PLAY;
            }

            else if (code == 5)
            {
                printf("Player %d - You LOST... :(\n", player_number);
                reset_board(300, 300, dim);
                read_board();

                bot_status = SEND_PLAY;
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
    int play[2];
    char str_play[3];
    int color[3];
    char buffer[BUFFER_SIZE];
  
   // recebe todos os dados da board
    while (strcmp(buffer, "board_sent") != 0)
    {
        memset(buffer, 0, BUFFER_SIZE);
        n = read(sock_fd, buffer, BUFFER_SIZE);
        if (n == -1)
        {
            perror("error reading cell state");
            exit(-1);
        }
        buffer[strlen(buffer)] = '\0';

        if (strcmp(buffer, "board_sent") != 0) // recebe a board toda a board 
        {
            sscanf(buffer, "%s %d %d %d %d %d", str_play, &color[0], &color[1], &color[2], &play[0], &play[1]);

            paint_card(play[0], play[1], color[0], color[1], color[2]);

            write_card(play[0], play[1], str_play, 200, 200, 200);
        }
    }
}


/***********************************************************************************
 * read_sdl_events()
 * 
 * Lê do teclado:
 *  _ se fechou a janela, acaba a sessao do jogador;
 * 
 * *********************************************************************************/
void *read_sdl_events()
{
    
    SDL_Event event;
    char buffer[BUFFER_SIZE] = {'\0'};

    while (end!=1)
    {
        while (SDL_PollEvent(&event))
        {
            if(event.type == SDL_QUIT)
            {
                // send message to server saying we're about to quit
                memset(buffer, 0, BUFFER_SIZE);
                strcpy(buffer, "exiting");
                printf("Im leaving the game!\n");
                write_payload(buffer, sock_fd);
                close_board_windows();
                end = 1;
                break;
            }
        }
    }

    return(NULL);
}


/***********************************************************************************
 * generate_first_play():
 * 
 *  Gera uma jogada aleatoria pelo bot (é first e second play)
 * 
 * ********************************************************************************/
void *generate_first_play(void *arg)
{
    int dim = *((int *)arg);
    char buffer[BUFFER_SIZE] = {'\0'};

    playable_place *random_place = (playable_place *)malloc(sizeof(playable_place));

    if (random_place == NULL)
    {
        printf("Memory Allocation of new position failed!\n");
        exit(1);
    }

    while (end!=1)
    {   
        if (bot_status == SEND_PLAY)
        {
            random_place->position[0] = rand() % dim;
            random_place->position[1] = rand() % dim;

            memset(buffer, 0, BUFFER_SIZE);
            sprintf(buffer, "%d %d", random_place->position[0], random_place->position[1]);
            write_payload(buffer, sock_fd);
            bot_status = WAITING_RESPONSE;
        }
        sleep(1);
    }
    pthread_exit(NULL);
}

/***********************************************************************************
 * main()
 * 
 * Faz a connecão com o servidor, recebe os parametros iniciais como a dimensao da
 * board, a cor associada e o numero de jogador.
 * 
 * Inicia as jogadas do jogador e recebe informação do servidor
 * 
 * *********************************************************************************/
int main(int argc, char *argv[])
{
    
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    pthread_t thread_ID_read_sdl_events;
    pthread_t thread_ID_generate_plays;

    int n = 0;
    dim = 0;
    end=0;

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
    buffer[strlen(buffer)]='\0';

    // recebe o numero de jogador associado, a dimensao da board, e as cores associadas
    sscanf(buffer, "%d %d %d %d %d", &player_number, &dim, &my_color[0], &my_color[1], &my_color[2]);

    printf("player number %d\n", player_number);
    printf("board dimension: %d\n", dim);
    printf("player color: [%d,%d,%d]\n", my_color[0], my_color[1], my_color[2]);

     // cria a tabela de jogo
    create_board_window(300, 300, dim);

    // posiciona as jogadas feitas
    read_board();
    printf("Received all the board info\n");

    // comeca a jogar
    pthread_create(&thread_ID_read_sdl_events, NULL, read_sdl_events, NULL); // change this cause function only reads SDL_QUIT

    bot_status = SEND_PLAY;
    pthread_create(&thread_ID_generate_plays, NULL, generate_first_play, (void *)&dim);
    
    // detecta as jogadas enviadas pelo servidor
    read_plays();

    close_board_windows();

    return 0;
}