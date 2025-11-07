#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h>
#include <windows.h>
#include "maze.h"

#define PLAYERS_FILE "players.txt"

typedef struct {
    char name[64];
    int best_easy;
    int best_easy_steps;
    int best_medium;
    int best_medium_steps;
    int best_hard;
    int best_hard_steps;
} Player;

typedef struct {
    int x, y;
} Pos;

// ========== UTILITIES ==========

static void set_color(int c) { SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), c); }

static void clear_screen() {
    COORD topLeft = {0, 0};
    DWORD written;
    CONSOLE_SCREEN_BUFFER_INFO info;
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(h, &info);
    FillConsoleOutputCharacterA(h, ' ', info.dwSize.X * info.dwSize.Y, topLeft, &written);
    FillConsoleOutputAttribute(h, info.wAttributes, info.dwSize.X * info.dwSize.Y, topLeft, &written);
    SetConsoleCursorPosition(h, topLeft);
}

static void pause_anykey() { printf("\nPress any key..."); _getch(); }

static void draw_char_at_console(int cx, int cy, int color, char ch) {
    COORD pos = {(SHORT)cx, (SHORT)cy};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
    putchar(ch);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
}
#define draw_char draw_char_at_console

// ========== PLAYER SYSTEM ==========

static Player *load_all_players(int *count) {
    *count = 0;
    FILE *f = fopen(PLAYERS_FILE, "r");
    if (!f) return NULL;
    int capacity = 8;
    Player *arr = malloc(sizeof(Player) * capacity);
    while (1) {
        Player p;
        if (fscanf(f, "%63s %d %d %d %d %d %d",
                   p.name,
                   &p.best_easy, &p.best_easy_steps,
                   &p.best_medium, &p.best_medium_steps,
                   &p.best_hard, &p.best_hard_steps) != 7)
            break;
        if (*count >= capacity) {
            capacity *= 2;
            arr = realloc(arr, sizeof(Player) * capacity);
        }
        arr[(*count)++] = p;
    }
    fclose(f);
    return arr;
}

static void save_all_players(Player *arr, int count) {
    FILE *f = fopen(PLAYERS_FILE, "w");
    if (!f) return;
    for (int i = 0; i < count; i++) {
        fprintf(f, "%s %d %d %d %d %d %d\n",
                arr[i].name,
                arr[i].best_easy, arr[i].best_easy_steps,
                arr[i].best_medium, arr[i].best_medium_steps,
                arr[i].best_hard, arr[i].best_hard_steps);
    }
    fclose(f);
}

static Player *find_player(Player *arr, int count, const char *name) {
    if (!arr) return NULL;
    for (int i = 0; i < count; i++)
        if (strcmp(arr[i].name, name) == 0) return &arr[i];
    return NULL;
}

// ========== MAZE DRAW ==========

static void draw_maze(Maze *m, Pos *p, int gx, int gy, int show_goal) {
    COORD coord = {0, 0};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    set_color(8);
    for (int i = 0; i < m->w + 2; i++) putchar('#');
    putchar('\n');
    for (int y = 0; y < m->h; y++) {
        set_color(8); putchar('#'); set_color(7);
        for (int x = 0; x < m->w; x++) {
            if (x == p->x && y == p->y) { set_color(10); putchar('@'); set_color(7); }
            else if (show_goal && x == gx && y == gy) { set_color(12); putchar('X'); set_color(7); }
            else putchar(m->grid[y * m->w + x]);
        }
        set_color(8); putchar('#'); putchar('\n'); set_color(7);
    }
    set_color(8);
    for (int i = 0; i < m->w + 2; i++) putchar('#');
    putchar('\n');
    set_color(7);
}

// ========== GAMEPLAY ==========

static int place_goal(Maze *m, int *gx, int *gy) {
    int startx = 1, starty = 1;
    int best_x = startx, best_y = starty;
    int best_dist = 0;

    // Search for a far cell (empty space)
    for (int y = 1; y < m->h - 1; y++) {
        for (int x = 1; x < m->w - 1; x++) {
            if (m->grid[y * m->w + x] == ' ') {
                int dx = x - startx;
                int dy = y - starty;
                int dist = dx * dx + dy * dy; // squared distance
                if (dist > best_dist) {
                    best_dist = dist;
                    best_x = x;
                    best_y = y;
                }
            }
        }
    }

    *gx = best_x;
    *gy = best_y;
    return 1;
}


static int play_level(Player *pl, int level) {
    int w = (level == 1 ? 30 : (level == 2 ? 50 : 60));
    int h = (level == 1 ? 20 : (level == 2 ? 30 : 40)); // Hard mode slightly shorter
    Maze *m = maze_create(w, h);
    maze_generate(m, level);
    Pos player = {1, 1};
    int gx = 0, gy = 0;
    place_goal(m, &gx, &gy);
    int moves = 0;
    clock_t tstart = clock();
    int quit = 0;

    while (1) {
        draw_maze(m, &player, gx, gy, 1);
        char ch = _getch();
        int nx = player.x, ny = player.y;
        if (ch == 'z') ny--;
        else if (ch == 's') ny++;
        else if (ch == 'q') nx--;
        else if (ch == 'd') nx++;
        else if (ch == 17) { quit = 1; break; } // Ctrl+Q
        if (nx >= 0 && ny >= 0 && nx < m->w && ny < m->h && m->grid[ny * m->w + nx] == ' ') {
            player.x = nx; player.y = ny; moves++;
        }
        if (player.x == gx && player.y == gy) break;
    }

    int elapsed = (int)((clock() - tstart) / CLOCKS_PER_SEC);
    int score = moves + elapsed;
    clear_screen();

    if (!quit && player.x == gx && player.y == gy) {
        printf("\nYou escaped the maze!\nTime: %ds  Steps: %d  Score: %d\n", elapsed, moves, score);

        if (level == 1) {
            if (pl->best_easy == 0 || score < pl->best_easy) { pl->best_easy = score; pl->best_easy_steps = moves; }
        } else if (level == 2) {
            if (pl->best_medium == 0 || score < pl->best_medium) { pl->best_medium = score; pl->best_medium_steps = moves; }
        } else {
            if (pl->best_hard == 0 || score < pl->best_hard) { pl->best_hard = score; pl->best_hard_steps = moves; }
        }

        while (1) {
            printf("\n1. Play Again\n2. Main Menu\n3. Exit\nChoice: ");
            char c = _getch();
            if (c == '1') { maze_destroy(m); return 2; }
            else if (c == '2') { maze_destroy(m); return 1; }
            else if (c == '3') { maze_destroy(m); exit(0); }
        }
    } else {
        printf("\nYou quit the maze. No score saved.\n");
        pause_anykey();
    }

    maze_destroy(m);
    return 1;
}

// ========== MENUS ==========

static int choose_level_menu() {
    clear_screen();
    printf("Select Level:\n");
    printf("1. Easy (30x20)\n");
    printf("2. Medium (50x30)\n");
    printf("3. Hard (65x40)\n");
    while (1) {
        char c = _getch();
        if (c == '1') return 1;
        if (c == '2') return 2;
        if (c == '3') return 3;
    }
}

static void show_high_scores(Player *players, int count) {
    clear_screen();
    printf("=== High Scores ===\n\n");
    for (int level = 3; level >= 1; level--) {
        if (level == 3) printf("---- HARD ----\n");
        else if (level == 2) printf("---- MEDIUM ----\n");
        else printf("---- EASY ----\n");

        int n = 0;
        Player *list = malloc(sizeof(Player) * count);
        for (int i = 0; i < count; i++) {
            int has = (level == 1 ? players[i].best_easy :
                       level == 2 ? players[i].best_medium :
                       players[i].best_hard);
            if (has > 0) list[n++] = players[i];
        }
        for (int i = 0; i < n; i++)
            for (int j = i + 1; j < n; j++) {
                int si = (level == 1 ? list[i].best_easy : level == 2 ? list[i].best_medium : list[i].best_hard);
                int sj = (level == 1 ? list[j].best_easy : level == 2 ? list[j].best_medium : list[j].best_hard);
                if (sj < si) { Player tmp = list[i]; list[i] = list[j]; list[j] = tmp; }
            }
        for (int i = 0; i < n; i++) {
            if (level == 1) printf("%s score: %d steps: %d\n", list[i].name, list[i].best_easy, list[i].best_easy_steps);
            else if (level == 2) printf("%s score: %d steps: %d\n", list[i].name, list[i].best_medium, list[i].best_medium_steps);
            else printf("%s score: %d steps: %d\n", list[i].name, list[i].best_hard, list[i].best_hard_steps);
        }
        free(list);
        printf("\n");
    }
    pause_anykey();
}

// ========== MAIN ==========

int main(void) {
    srand((unsigned)time(NULL));
    int player_count = 0;
    Player *players = load_all_players(&player_count);
    int selected_level = 1;

    while (1) {
        clear_screen();
        printf("=== MAZE GAME ===\n");
        printf("1. New Player\n");
        printf("2. Continue\n");
        printf("3. Select Level\n");
        printf("4. High Scores\n");
        printf("5. Quit\n");
        char choice = _getch();

        if (choice == '5') {
            save_all_players(players, player_count);
            break;
        } else if (choice == '4') {
            show_high_scores(players, player_count);
        } else if (choice == '3') {
            selected_level = choose_level_menu();
        } else if (choice == '1' || choice == '2') {
            clear_screen();
            printf("Enter player name: ");
            char name[64];
            scanf("%63s", name);
            Player *pl = find_player(players, player_count, name);

            if (choice == '1') {
                if (pl) {
                    printf("\nPlayer already exists! Press any key...");
                    _getch();
                    continue;
                }
                Player np = {0};
                strcpy(np.name, name);
                players = realloc(players, sizeof(Player) * (player_count + 1));
                players[player_count++] = np;
                pl = &players[player_count - 1];
                save_all_players(players, player_count);
            } else {
                if (!pl) {
                    printf("\nNo player found with that name! Press any key...");
                    _getch();
                    continue;
                }
            }

            while (1) {
                int res = play_level(pl, selected_level);
                if (res == 1) { save_all_players(players, player_count); break; }
                else if (res == 2) continue;
            }
        }
    }

    if (players) free(players);
    return 0;
}
