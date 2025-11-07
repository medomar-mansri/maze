#ifndef MAZE_H
#define MAZE_H

typedef struct {
    int w;
    int h;
    char *grid;
} Maze;

Maze *maze_create(int w, int h);
void maze_destroy(Maze *m);
void maze_generate(Maze *m, int difficulty);  
void maze_print(Maze *m);
int maze_index(Maze *m, int x, int y);

#endif
