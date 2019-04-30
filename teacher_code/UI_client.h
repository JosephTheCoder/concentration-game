
#define CONCENTRATION_GAME_PORT 3000

typedef struct player{
  int player_fd;
  char color;
} player;

int fd, newfd, dim;
char buffer[128];
ssize_t n;
socklen_t addrlen;
struct addrinfo hints, *res;
struct sockaddr_in addr;
