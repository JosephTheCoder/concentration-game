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

int end;

typedef struct memory_place{
  char v[3]; 
  int position[2]; // [r, g, b]
  struct memory_place *next;
} memory_place;

typedef struct playable_place{
  int position[2]; // [r, g, b]
  struct playable_place *next;
} playable_place;


#define CONCENTRATION_GAME_PORT 3026
#define BUFFER_SIZE 128

#define SEND_PLAY 1
#define WAITING_RESPONSE 2
#define IDLE 3

#define MAX_POSITIONS_IN_MEMORY 6


int sock_fd = 0;
int dim = 0, n = 0;
int terminate = 0;

int bot_status = IDLE;

int player_number;

int my_color[3];
int background_color[3] = {255, 255, 255};
int text_color[3] = {200, 200, 200};

int nr_playable_positions = 0;
int nr_memory_positions = 0;

memory_place *bot_memory = NULL; // saves the last played positions info
playable_place *playable_positions = NULL; // saves playable positions


int write_payload(char *, int);
void read_plays();
void read_board();
void *read_sdl_events();
void *generate_first_play();
void save_playable_position(int *new_position);
void remove_playable_position(int *position);
playable_place *get_playable_position(int index);
memory_place *find_relative_in_memory(char *letters);
void save_in_memory(char *letters, int *position);