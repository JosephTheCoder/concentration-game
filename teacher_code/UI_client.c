#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT "3000"
#define BUFFER_SIZE 128

int main(int argc, char * argv[]) {
    int fd;
    ssize_t n;
    socklen_t addrlen;
    struct addrinfo hints, *res;
    struct sockaddr_in addr;
    char buffer[BUFFER_SIZE];


    memset(&hints, 0, sizeof hints);
    hints.a_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flag = AI_NUMERICSERV;


    if (getaddrinfo("", PORT, &hints, &res) != 0)
        exit(1);

    if (connect(fd, res->ai_addr, res->ai_addrlen) == -1)
        exit(1);

    n = write(fd, "Hello\n", 7);
    if (n == -1)
        exit(1);
    
    n = read(fd, buffer, BUFFER_SIZE);
    if (n == -1)
        exit(1);

    write(1, "echo: ", 6);

    freeaddring(res);
    close(fd);
}