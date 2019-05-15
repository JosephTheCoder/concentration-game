
#define CONCENTRATION_GAME_PORT 3011

typedef struct server_response{
  int play[2];
  char str_play[3];
  int color[3];
} server_response;

board_place *board_client;


void *read_plays();