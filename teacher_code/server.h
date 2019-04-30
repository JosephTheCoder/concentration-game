#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

#define CONCENTRATION_GAME_PORT 3000
#define MAX_PLAYERS 10

int fd, newfd;
char buffer[128];
ssize_t n;
socklen_t addrlen;
struct addrinfo hints, *res;
struct sockaddr_in addr;

typedef struct jogador{
    int color[3];
    int num;
    int fd;
}jogador;

void server_fcn();
char rand_color();
void * thread_fcn();
