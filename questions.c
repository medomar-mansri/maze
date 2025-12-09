#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "questions.h"
#include "win_utils.h" 

// Question structure used internally (not stored to disk)
typedef struct {
    char text[256];   // code snippet + question
    char choices[3][64];
    char correct;     // 'A', 'B', 'C'
} QGen;

// utility: shuffle int array
static void shuffle_int(int *a, int n) {
    for (int i = n-1; i > 0; --i) {
        int j = rand() % (i+1);
        int t = a[i]; a[i] = a[j]; a[j] = t;
    }
}

// create an array-index question (values and index randomized)
static QGen gen_array_question() {
    QGen q;
    int vals[5];
    // random distinct multiples of 5 between 5 and 50
    for (int i = 0; i < 5; ++i) vals[i] = (rand() % 10 + 1) * 5;
    int idx = rand() % 5;
    snprintf(q.text, sizeof(q.text),
             "int t[5] = {%d, %d, %d, %d, %d};\nprintf(\"%%d\", t[%d]);",
             vals[0], vals[1], vals[2], vals[3], vals[4], idx);

    int correct_val = vals[idx];

    // create two distractors (neighboring index values or random)
    int d1 = vals[(idx + 1) % 5];
    int d2 = vals[(idx + 2) % 5];

    // assemble choices and shuffle them so correct position changes
    int perm[3] = {0,1,2};
    shuffle_int(perm, 3);

    int choice_vals[3] = { correct_val, d1, d2 };
    for (int i = 0; i < 3; ++i) {
        snprintf(q.choices[i], sizeof(q.choices[i]), "%d", choice_vals[perm[i]]);
    }

    // find which position has the correct value
    for (int i = 0; i < 3; ++i) {
        if (strcmp(q.choices[i], (char[]){0}) == 0) {} // nothing, just avoid warnings
    }
    for (int i = 0; i < 3; ++i) {
        if (atoi(q.choices[i]) == correct_val) {
            q.correct = (char)('A' + i);
            break;
        }
    }
    return q;
}

// char arithmetic question
static QGen gen_char_question() {
    QGen q;
    // pick a letter between 'A' and 'X' so +1 is visible letter
    char base = (char)('A' + (rand() % 23));
    snprintf(q.text, sizeof(q.text),
             "char c = '%c';\nprintf(\"%%c\", c + 1);", base);

    char correct_char = (char)(base + 1);
    char d1 = (char)(base - 1);
    char d2 = (char)(base + 2);

    char opts[3][4];
    snprintf(opts[0], sizeof(opts[0]), "%c", correct_char);
    snprintf(opts[1], sizeof(opts[1]), "%c", d1);
    snprintf(opts[2], sizeof(opts[2]), "%c", d2);

    int perm[3] = {0,1,2};
    shuffle_int(perm, 3);
    for (int i = 0; i < 3; ++i) snprintf(q.choices[i], sizeof(q.choices[i]), "%s", opts[perm[i]]);

    for (int i = 0; i < 3; ++i) if (q.choices[i][0] == correct_char) { q.correct = 'A' + i; break; }
    return q;
}

// pointer math question
static QGen gen_pointer_question() {
    QGen q;
    // create small array with 4 elements
    int a = (rand() % 5 + 1) * 2;
    int b = (rand() % 5 + 2) * 2;
    int c = (rand() % 5 + 3) * 2;
    int d = (rand() % 5 + 4) * 2;
    int vals[4] = {a,b,c,d};
    int offset = 1 + (rand() % 2); // 1 or 2
    snprintf(q.text, sizeof(q.text),
             "int a[] = {%d, %d, %d, %d};\nint *p = &a[1];\nprintf(\"%%d\", *(p + %d));",
             vals[0], vals[1], vals[2], vals[3], offset);

    int correct_val = vals[1 + offset]; // since p = &a[1]

    // distractors
    int d1 = vals[(1 + offset + 1) % 4];
    int d2 = vals[(1 + offset + 2) % 4];

    int choice_vals[3] = {correct_val, d1, d2};
    int perm[3] = {0,1,2};
    shuffle_int(perm, 3);
    for (int i = 0; i < 3; ++i) snprintf(q.choices[i], sizeof(q.choices[i]), "%d", choice_vals[perm[i]]);
    for (int i = 0; i < 3; ++i) if (atoi(q.choices[i]) == correct_val) { q.correct = 'A' + i; break; }
    return q;
}

// x++ print question
static QGen gen_postinc_question() {
    QGen q;
    int x = 1 + rand() % 9; // 1..9
    snprintf(q.text, sizeof(q.text),
             "int x = %d;\nprintf(\"%%d\", x++);", x);
    int correct_val = x;       // prints original
    int d1 = x + 1;
    int d2 = x - 1;
    int choice_vals[3] = {correct_val, d1, d2};
    int perm[3] = {0,1,2};
    shuffle_int(perm, 3);
    for (int i = 0; i < 3; ++i) snprintf(q.choices[i], sizeof(q.choices[i]), "%d", choice_vals[perm[i]]);
    for (int i = 0; i < 3; ++i) if (atoi(q.choices[i]) == correct_val) { q.correct = 'A' + i; break; }
    return q;
}

// pick random generator by difficulty (slightly bias type by difficulty)
static QGen generate_for_level(int level) {
    int r;
    if (level == 1) { // easy: array, postinc, char
        r = rand() % 3;
        if (r == 0) return gen_array_question();
        if (r == 1) return gen_postinc_question();
        return gen_char_question();
    } else if (level == 2) { // medium: array, pointer, strlen simulated via string literal
        r = rand() % 3;
        if (r == 0) return gen_array_question();
        if (r == 1) return gen_pointer_question();
        // simulate a simple string question using postinc-style
        return gen_char_question();
    } else { // hard (but still basic): pointer, array, postinc
        r = rand() % 3;
        if (r == 0) return gen_pointer_question();
        if (r == 1) return gen_array_question();
        return gen_postinc_question();
    }
}

// main API: ask 3 questions, returns total adjustment seconds
int ask_questions(int level) {
    // level: 1=easy,2=medium,3=hard
    int correct_bonus = 0, wrong_penalty = 0;
    if (level == 1) { correct_bonus = 10; wrong_penalty = -5; }
    else if (level == 2) { correct_bonus = 10; wrong_penalty = -10; }
    else { correct_bonus = 5; wrong_penalty = -15; }

    // ensure randomness per-call
    // (caller already seeded srand, but keep safe)
    // srand((unsigned)time(NULL)); // DO NOT reseed here (main seeds once)

    QGen used[3];
    int used_count = 0;
    int total_adj = 0;

    for (int i = 0; i < 3; ++i) {
        QGen q;
        // generate unique question (avoid duplicates in these 3)
        int attempts = 0;
        while (1) {
            q = generate_for_level(level);
            int dup = 0;
            for (int u = 0; u < used_count; ++u) {
                if (strcmp(used[u].text, q.text) == 0) { dup = 1; break; }
            }
            if (!dup) break;
            attempts++;
            if (attempts > 8) break;
        }
        used[used_count++] = q;

        clear_screen();
        printf("Question %d/3:\n\n", i + 1);
        printf("%s\n\n", q.text);
        printf("A) %s\n", q.choices[0]);
        printf("B) %s\n", q.choices[1]);
        printf("C) %s\n\n", q.choices[2]);

        printf("Your answer (A/B/C): ");
        char ch = 0;
        while (1) {
            ch = _getch();
            if (ch == 'a' || ch == 'A' || ch == 'b' || ch == 'B' || ch == 'c' || ch == 'C') break;
        }
        char ans = (ch >= 'a' && ch <= 'z') ? (ch - ('a' - 'A')) : ch;
        if (ans == q.correct) {
            printf("\nCorrect! +%d seconds\n", correct_bonus);
            total_adj += correct_bonus;
        } else {
            printf("\nWrong! %d seconds (Correct: %c)\n", wrong_penalty, q.correct);
            total_adj += wrong_penalty;
        }
        Sleep(900);
    }

    // final safety clamp will be applied by caller (main), but we keep the returned value as-is
    return total_adj;
}
