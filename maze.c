#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "maze.h"

static int rand_range(int a, int b) {
    return a + rand() % (b - a + 1);
}

Maze *maze_create(int w, int h) {
    Maze *m = malloc(sizeof(Maze));
    if (!m) return NULL;
    m->w = w;
    m->h = h;
    m->grid = malloc(w * h);
    if (!m->grid) { free(m); return NULL; }
    for (int i = 0; i < w * h; ++i) m->grid[i] = '#';
    return m;
}

void maze_destroy(Maze *m) {
    if (!m) return;
    free(m->grid);
    free(m);
}

int maze_index(Maze *m, int x, int y) {
    return y * m->w + x;
}

static void carve(Maze *m, int x, int y) {
    int dirs[4][2] = { {1,0}, {-1,0}, {0,1}, {0,-1} };
    for (int i = 0; i < 4; ++i) {
        int r = rand() % 4;
        int tx = dirs[i][0], ty = dirs[i][1];
        dirs[i][0] = dirs[r][0]; dirs[i][1] = dirs[r][1];
        dirs[r][0] = tx; dirs[r][1] = ty;
    }
    for (int i = 0; i < 4; ++i) {
        int nx = x + dirs[i][0] * 2;
        int ny = y + dirs[i][1] * 2;
        if (nx > 0 && ny > 0 && nx < m->w-1 && ny < m->h-1 && m->grid[maze_index(m, nx, ny)] == '#') {
            m->grid[maze_index(m, nx - dirs[i][0], ny - dirs[i][1])] = ' ';
            m->grid[maze_index(m, nx, ny)] = ' ';
            carve(m, nx, ny);
        }
    }
}

void maze_generate(Maze *m, int difficulty) {
    m->grid[maze_index(m, 1, 1)] = ' ';
    carve(m, 1, 1);

    int extra_breaks = 0;
    if (difficulty == 0) extra_breaks = (m->w * m->h) / 15;  // easy
    else if (difficulty == 1) extra_breaks = (m->w * m->h) / 30;  // medium
    else extra_breaks = 0;  // hard

    for (int i = 0; i < extra_breaks; ++i) {
        int x = rand_range(1, m->w - 2);
        int y = rand_range(1, m->h - 2);
        if (m->grid[maze_index(m, x, y)] == '#') {
            m->grid[maze_index(m, x, y)] = ' ';
        }
    }
}

void maze_print(Maze *m) {
    for (int y = 0; y < m->h; ++y) {
        for (int x = 0; x < m->w; ++x)
            putchar(m->grid[maze_index(m, x, y)]);
        putchar('\n');
    }
}
