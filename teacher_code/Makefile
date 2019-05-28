
CC=gcc
CFLAGS= -Wall -g

SRCFILES_server = server.c board_library.c
INCFILES_server = server.h board_library.h
OBJFILES_server = server.o

TARGET_server = server
TARGET_client = client
TARGET_bot = bot

SRCFILES_client = UI_client.c UI_library.c
INCFILES_client = UI_client.h UI_library.h
OBJFILES_client = client.o

SRCFILES_bot = UI_bot.c UI_library.c
INCFILES_bot = UI_bot.h UI_library.h
OBJFILES_bot = bot.o

server: $(TARGET_server)

$(TARGET_server): $(OBJFILES_server)
	$(CC) $(CFLAGS) $(SRCFILES_server) -o $(TARGET_server) -lm -pthread -lSDL2 -lSDL2_ttf

clean_server:
	rm -f $(TARGET_server) $(OBJFILES_client) core.* #*

client: $(TARGET_client)

$(TARGET_client): $(OBJFILES_client)
	$(CC) $(CFLAGS) $(SRCFILES_client) -o $(TARGET_client) -lm -pthread -lSDL2 -lSDL2_ttf

clean_client:
	rm -f $(TARGET_client) $(OBJFILES_client) core.* #*

server.o: server.c server.h board_library.h

client.o: UI_client.c board_library.h UI_client.h UI_library.h

bot: $(TARGET_bot)

$(TARGET_bot): $(OBJFILES_bot)
	$(CC) $(CFLAGS) $(SRCFILES_bot) -o $(TARGET_bot) -lm -pthread -lSDL2 -lSDL2_ttf

clean_bot:
	rm -f $(TARGET_bot) $(OBJFILES_bot) core.* #*

bot.o: UI_bot.c board_library.h UI_bot.h UI_library.h