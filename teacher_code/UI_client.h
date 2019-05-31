
#define CONCENTRATION_GAME_PORT 3019

board_place *board_client;

int player_number;

int my_color[3];
int background_color[3] = {255, 255, 255};
int text_color[3] = {200, 200, 200};


void read_plays();
void read_board();