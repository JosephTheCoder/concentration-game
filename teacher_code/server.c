#define PORT "3000"

#include "board_library.h"
#include "UI_library.h"
#include "server.h"


void * thread_fcn(void* arg){

    while(1){
        n=read(newfd, buffer, 129);
        if(n==-1)
            exit(1);

        write(1, "received: ", 10);
        write(1, buffer, n);    
    }
}


void server_fcn(int argc, char* argv[]){

    memset(&hints, 0, sizeof hints);
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_STREAM;;
    hints.ai_flags=AI_PASSIVE|AI_NUMERICSERV;
    pthread_t * thread_ID;
    int dim=0;

    if(argc!=2 || sscanf(argv[1], "%d", &dim)==0){
        printf("Please provide a correct dimension argument.\n");
        exit(1);
    }


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

        write(1, dim, sizeof(dim));   

        pthread_create(&thread_ID,NULL, thread_fcn,NULL);

    }

    freeaddrinfo(res);
    close(newfd);
    close(fd);
}



