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

typedef struct playable_place{
  int position[2]; // [r, g, b]
  struct playable_place *next;
} playable_place;

#define CONCENTRATION_GAME_PORT 3031
#define BUFFER_SIZE 128

#define SEND_PLAY 1
#define WAITING_RESPONSE 2
#define IDLE 3

#define MAX_POSITIONS_IN_MEMORY 6

int end;

int sock_fd = 0;
int dim = 0, n = 0;
int terminate = 0;

int bot_status = IDLE;

int player_number;

int my_color[3];
int background_color[3] = {255, 255, 255};
int text_color[3] = {200, 200, 200};

int write_payload(char *, int);
void read_plays();
void read_board();
void *read_sdl_events();
void *generate_first_play();