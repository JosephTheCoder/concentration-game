#include <stdlib.h>
#include "board_library.h"
#include <stdio.h>
#include <string.h>


play_response resp[100];
int cancel[100];

int linear_conv(int i, int j)
{
  return j * dim_board + i;
}
char *get_board_place_str(int i, int j)
{
  return board[linear_conv(i, j)].v;
}

int get_card_state(int i, int j)
{
  return board[linear_conv(i, j)].state;
}

void init_board(int dim)
{
  int count = 0;
  int i, j;
  char *str_place;

  dim_board = dim;
  n_corrects = 0;

  for(i=0;i<100;i++)
      play1[i][0] = -1;

  board = malloc(sizeof(board_place) * dim * dim);

  for (i = 0; i < (dim_board * dim_board); i++)
  {
    board[i].v[0] = '\0';
    board[i].color[0] = 255;
    board[i].color[1] = 255;
    board[i].color[2] = 255;
    board[i].state=0;
  }

  for (char c1 = 'a'; c1 < ('a' + dim_board); c1++)
  {
    for (char c2 = 'a'; c2 < ('a' + dim_board); c2++)
    {
      do
      {
        i = random() % dim_board;
        j = random() % dim_board;
        str_place = get_board_place_str(i, j);
        printf("%d %d -%s-\n", i, j, str_place);
      } while (str_place[0] != '\0');
      
      str_place[0] = c1;
      str_place[1] = c2;
      str_place[2] = '\0';
      do
      {
        i = random() % dim_board;
        j = random() % dim_board;
        str_place = get_board_place_str(i, j);
        printf("%d %d -%s-\n", i, j, str_place);
      } while (str_place[0] != '\0');
      str_place[0] = c1;
      str_place[1] = c2;
      str_place[2] = '\0';
      count += 2;
      if (count == dim_board * dim_board)
        return;
    }
  }
}

play_response board_play(int x, int y, int fd, int cancela)
{
  cancel[fd]=cancela;
  
  if(cancel[fd]==1){
    resp[fd].code=0;
    play1[fd][0] = -1;  
    play1[fd][1] = -1;
  }
  else{
    resp[fd].code = 10;
    if (get_card_state(x, y)==1)
    {
      printf("FILLED\n");
      resp[fd].code = 0;
    }
    else
    {
      if (play1[fd][0] == -1)
      {
        printf("FIRST\n");
        resp[fd].code = 1;

        play1[fd][0] = x;
        play1[fd][1] = y;
        resp[fd].play1[0] = play1[fd][0];
        resp[fd].play1[1] = play1[fd][1];
        strcpy(resp[fd].str_play1, get_board_place_str(x, y));
      }
      else
      {

        char *first_str = get_board_place_str(play1[fd][0], play1[fd][1]);
        char *secnd_str = get_board_place_str(x, y);

        if ((play1[fd][0] == x) && (play1[fd][1] == y))
        {
          resp[fd].code = 0;
          printf("FILLED\n");
        }
        else
        {
          resp[fd].play1[0] = play1[fd][0];
          resp[fd].play1[1] = play1[fd][1];
          strcpy(resp[fd].str_play1, first_str);
          resp[fd].play2[0] = x;
          resp[fd].play2[1] = y;
          strcpy(resp[fd].str_play2, secnd_str);

          if (strcmp(first_str, secnd_str) == 0)
          {
            printf("CORRECT!!!\n");
            n_corrects += 2;
            if (n_corrects == dim_board * dim_board)
              resp[fd].code = 3;
            else
              resp[fd].code = 2;
          }
          else
          {
            printf("INCORRECT");

            resp[fd].code = -2;
          }
          play1[fd][0] = -1;
        }
      }
    }
  }
  return resp[fd];
}

void restart_board(board_place *board, int dim)
{
  int count = 0;
  int i, j;
  char *str_place;

  dim_board = dim;
  n_corrects = 0;

  for(i=0;i<100;i++)
      play1[i][0] = -1;

  for (i = 0; i < (dim_board * dim_board); i++)
  {
    board[i].v[0] = '\0';
    board[i].color[0] = 255;
    board[i].color[1] = 255;
    board[i].color[2] = 255;
    board[i].state=0;
  }

  for (char c1 = 'a'; c1 < ('a' + dim_board); c1++)
  {
    for (char c2 = 'a'; c2 < ('a' + dim_board); c2++)
    {
      do
      {
        i = random() % dim_board;
        j = random() % dim_board;
        str_place = get_board_place_str(i, j);
        printf("%d %d -%s-\n", i, j, str_place);
      } while (str_place[0] != '\0');
      
      str_place[0] = c1;
      str_place[1] = c2;
      str_place[2] = '\0';
      do
      {
        i = random() % dim_board;
        j = random() % dim_board;
        str_place = get_board_place_str(i, j);
        printf("%d %d -%s-\n", i, j, str_place);
      } while (str_place[0] != '\0');
      str_place[0] = c1;
      str_place[1] = c2;
      str_place[2] = '\0';
      count += 2;
      if (count == dim_board * dim_board)
        return;
    }
  }
}
