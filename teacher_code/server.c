#define PORT "3000"

#include "board_library.h"
#include "UI_library.h"
#include "server.h"

#include <time.h>


char rand_color(){
            int x=0;
            char color[11]={'\0'};

             x=rand()%255; 
             strcat(color , x);
             strcat(color , "/");
             x=rand()%255; 
             strcat(color , x);
             strcat(color , "/");
             x=rand()%255; 
             strcat(color , x);
            return color;     
        } 

void * thread_fcn(void * arg){
  int nfd=*((int*)arg);

    while(1){
        n=read(nfd, buffer, 129);
        if(n==-1)
            exit(1);

        write(nfd, "connected: " , strlen("connected: "));
        write(1, buffer, n);    
    }
}


void main(int argc, char* argv[]){

    memset(&hints, 0, sizeof hints);
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_STREAM;;
    hints.ai_flags=AI_PASSIVE|AI_NUMERICSERV;
    pthread_t thread_ID;
    int dim=0;
    int nb_players=0;
    char color[11]={'\0'};
    srand(time(NULL));

    if(argc!=2 || sscanf(argv[1], "%d", &dim)==0){
        printf("Please provide a correct dimension argument.\n");
        exit(1);
    }
    init_board(dim); //mudar parametros de board
    
    n=getaddrinfo(NULL, PORT, &hints, &res);
    if(n!=0)
        exit(1);

    fd=socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(fd==-1)
        exit(1);

    n=bind(fd, res->ai_addr, res->ai_addrlen);
    if(n==-1)
        exit(1);

    if(listen(fd,10)==-1)
        exit(1);


    while(1){

    if((newfd==accept(fd,(struct sockaddr*)&addr, &addrlen))==-1) 
        exit(1);

        nb_players++;
        write(newfd, &dim, sizeof(dim)); 
        stcpy(color,rand_color());
        write(newfd,"your color code is: ", strlen("your color code is: ")); 
        write(newfd, color, strlen(color));

        if(nb_players<2){

            write(newfd, "Not enough players to start a game.\nPlease wait...", strlen("Not enough players to start a game.\nPlease wait..."));   
        }
        else{

            srcpty(buffer, board.v);
            strcat(buffer, color);

            for(i=0; i<(dim^2); i++){

                write(newfd, buffer, strlen(buffer));
            }
            pthread_create(&thread_ID, NULL, thread_fcn, (int *)newfd);
        }
    }

    freeaddrinfo(res);
    close(newfd);
    close(fd);
}



