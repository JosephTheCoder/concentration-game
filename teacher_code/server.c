#define PORT "3000"

#include "board_library.h"
#include "UI_library.h"
#include "server.h"

#include <time.h>


char rand_color()
{
    int x = 0;
    char color[11] = {'\0'};

    x = rand() % 255;
    strcat(color, x);
    strcat(color, "/");
    x = rand() % 255;
    strcat(color, x);
    strcat(color, "/");
    x = rand() % 255;
    strcat(color, x);
    return color;
}

void *thread_fcn(void *arg)
{
    int nfd = *((int *)arg);
   //sleep(5);

    while (1)
    {
        n = read(nfd, buffer, 129);
        if (n == -1)
            exit(1);

        
        write(nfd, "connected: ", strlen("connected: "));
        write(1, buffer, n);
    }
}

void main(int argc, char *argv[])
{
    struct sockaddr_in local_addr;
    
    pthread_t thread_ID;
    int dim = 0;
    int nb_players = 0;
    char color[11] = {'\0'};
    srand(time(NULL));
    

    if (argc != 2 || sscanf(argv[1], "%d", &dim) == 0)
    {
        printf("Please provide a correct dimension argument.\n");
        exit(1);
    }

    init_board(dim);  // por cores a preto [0,0,0]

    int sock_fd= socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1){
        perror("socket: ");
        exit(-1);
    }

    local_addr.sin_family = AF_INET;
    local_addr.sin_port= htons(CONCENTRATION_GAME_PORT);
    local_addr.sin_addr.s_addr= INADDR_ANY;   

    n = bind(sock_fd, (struct sockaddr *)&local_addr, sizeof(local_addr));
    if(n == -1) {
        perror("bind");
        exit(-1);
    }

    if (listen(sock_fd, 10) == -1)
        exit(1);

    while (1)
    {

        if ((newfd == accept(sock_fd, NULL, NULL)) == -1)
            exit(1);

        nb_players++;
        write(newfd, &nb_players, sizeof(nb_players));

        write(newfd, &dim, sizeof(dim));
        stcpy(color, rand_color());
        write(newfd, "your color code is: ", strlen("your color code is: "));
        write(newfd, color, strlen(color));


        while(nb_players<2){
            wait();
        }
        
        srcpty(buffer, board[i].v);
        strcat(buffer, color);

        for (i = 0; i < (dim ^ 2); i++)
        {
            if (board[i].color[0]!=107 &&  board[1].color[2]!=200 && board[3].color[3]!=100)
            {
                strcpy(buffer, board[i].v);
                strcat(buffer, "/");
                sprintf(color, "%d", board[i].color[0]);
                strcat(buffer,color);
                strcat(buffer, "/");
                sprintf(color, "%d", board[i].color[1]);
                strcat(buffer,color);
                strcat(buffer, "/");
                sprintf(color, "%d", board[i].color[2]);
                strcat(buffer,color);
                strcat(buffer, "/");
                strcat(buffer, i);

                write(newfd, buffer, strlen(buffer));

            }
        }
        pthread_create(&thread_ID, NULL, thread_fcn, (int *)newfd);
    }

    freeaddrinfo(res);
    close(newfd);
    close(fd);
}
