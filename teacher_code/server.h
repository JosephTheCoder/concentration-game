#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

#define CONCENTRATION_GAME_PORT 3013
#define BUFFER_SIZE 128

typedef struct player{
    int color[3]; //[r,g,b]
    int number;
    int fd;
    struct player *next;
} player_t;

int dim;
char buffer[BUFFER_SIZE];

void server_fcn();
void * thread_fcn();

int *random_color();
void send_state_board(int fd);
void push_to_list(player_t *head, int *color, int fd);
int remove_from_list(player_t **head, int number);
