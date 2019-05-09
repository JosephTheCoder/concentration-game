#
#* File Name: Makefile
#* Author:    Gonçalo Lemos / Rodrigo Carnall
#*

CC=gcc
CFLAGS= -Wall -g

SRCFILES_server = server.c board_library.c
INCFILES_server = server.h board_library.h
OBJFILES_server = server.o

TARGET_server = server
TARGET_client = client

<<<<<<< HEAD
SRCFILES_client = UI_client.c
INCFILES_client = board_library.h UI_client.h UI_library.h
=======
SRCFILES_client = UI_client.c UI_library.c
INCFILES_client = UI_client.h UI_library.h
>>>>>>> 4dbf551fcff7fd114fea52fc45f16e88ea8a7a08
OBJFILES_client = client.o

server: $(TARGET)

$(TARGET_server): $(OBJFILES_server)
	$(CC) $(CFLAGS) $(SRCFILES_server) -o $(TARGET_server) -lm -pthread -lSDL2 -lSDL2_ttf

clean_server:
	rm -f $(TARGET_server) $(OBJFILES_client) core.* #*

client: $(TARGET_client)

$(TARGET_client): $(OBJFILES_client)
	$(CC) $(CFLAGS) $(SRCFILES_client) -o $(TARGET_client) -lm -pthread -lSDL2 -lSDL2_ttf

clean_client:
	rm -f $(TARGET_client) $(OBJFILES_client) core.* #*

<<<<<<< HEAD
server.o: server.c board_library.h server.h  UI_library.h 
=======
server.o: server.c server.h board_library.h
>>>>>>> 4dbf551fcff7fd114fea52fc45f16e88ea8a7a08

client.o: UI_client.c board_library.h UI_client.h UI_library.h