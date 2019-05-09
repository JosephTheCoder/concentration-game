#
#* File Name: Makefile
#* Author:    Gonçalo Lemos / Rodrigo Carnall
#*

CC=gcc
CFLAGS= -Wall -g

SRCFILES_server = server.c
INCFILES_server = board_library.h server.h  UI_library.h
OBJFILES_server = server.o

TARGET_server = server
TARGET_client = client

SRCFILES_client = UI_client.c
INCFILES_client = board_library.h UI_client.h UI_library.h
OBJFILES_client = client.o

server: $(TARGET)

$(TARGET_server): $(OBJFILES_server)
	$(CC) $(CFLAGS) $(SRCFILES_server) -o $(TARGET_server) -lm -pthread

clean_server:
	rm -f $(TARGET_server) $(OBJFILES_client) core.* #*

client: $(TARGET_client)

$(TARGET_client): $(OBJFILES_client)
	$(CC) $(CFLAGS) $(SRCFILES_client) -o $(TARGET_client) -lm -pthread

clean_client:
	rm -f $(TARGET_client) $(OBJFILES_client) core.* #*

server.o: server.c board_library.h server.h  UI_library.h 

client.o: UI_client.c board_library.h UI_client.h UI_library.h