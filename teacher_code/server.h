#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

int fd, newfd;
char buffer[128];
ssize_t n;
socklen_t addrlen;
struct addrinfo hints, *res;
struct sockaddr_in addr;

void server_fcn();
void * thread_fcn();