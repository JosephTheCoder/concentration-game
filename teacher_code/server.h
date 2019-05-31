#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

#define CONCENTRATION_GAME_PORT 3027
#define BUFFER_SIZE 128

typedef struct player{
    int color[3]; //[r,g,b]
    int number;
    int fd;
    int nr_points;
    struct player *next;
} player_t;

int dim;
int nr_players = 0; // indica o numero DE jogadores

int  translate_i_to_x();
int  translate_i_to_y();
char build_resp();
void *read_first_play();
void *read_second_play();
char *create_winners_payload();
void server_fcn();
void * thread_fcn();
int translate_i_to_x(int, int);
int translate_i_to_y(int, int);

player_t * find_fd_list();
void update_cell_color(int, int, int, int, int, int);
int *random_color();
void *read_second_play(void *);
void *send_play_to_board(void *);
void *send_play_to_all(void *);
void *read_first_play(void *);
void send_state_board(int, int);
void push_to_list(player_t *, int *, int, int);
int remove_from_list( int);
