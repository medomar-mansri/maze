CC=gcc
CFLAGS=-std=c11 -O2 -Wall
SOURCES=main.c maze.c player.c
OBJECTS=$(SOURCES:.c=.o)
EXEC=echoes

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(EXEC) $(OBJECTS)
