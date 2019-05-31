
#define CONCENTRATION_GAME_PORT 3015

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


#define BUFFER_SIZE 128

int sock_fd;  // sock_fd do servidor
int dim , n;  // dimensao da board e nº bytes msgs

int done; // acaba o jogo do jogador quando =1

board_place *board_client;

int player_number;

int my_color[3];
int background_color[3] = {255, 255, 255};
int text_color[3] = {200, 200, 200};


void read_plays();
void read_board();