#ifndef MAZE_H
#define MAZE_H

typedef struct {
    int w;
    int h;
    char *grid;
} Maze;

Maze *maze_create(int w, int h);
void maze_destroy(Maze *m);
void maze_generate(Maze *m, int difficulty);  // difficulty: 0=easy,1=medium,2=hard
void maze_print(Maze *m);
int maze_index(Maze *m, int x, int y);

#endif
