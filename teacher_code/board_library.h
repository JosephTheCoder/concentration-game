#include <stdlib.h>

typedef struct board_place{
  char v[3]; 
  int color[3];
  int state;
} board_place;

typedef struct play_response{
  int code; // 0 - filled
            // 1 - 1st play
            // 2 2nd - same plays
            // 3 END
            // -2 2nd - diffrent
  int play1[2];
  int play2[2];
  char str_play1[3], str_play2[3];
} play_response;

int dim_board;
board_place *board;
int play1[100][2];
int n_corrects;

char * get_board_place_str(int, int);
int get_card_state(int , int );
void init_board(int);
play_response board_play (int, int, int, int);
int linear_conv(int, int);
