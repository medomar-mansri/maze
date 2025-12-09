CC=gcc
CFLAGS=-std=c11 -O2 -Wall
SOURCES=main.c maze.c questions.c win_utils.c

OBJECTS=$(SOURCES:.c=.o)
EXEC=echoes.exe

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(EXEC) $(OBJECTS)
