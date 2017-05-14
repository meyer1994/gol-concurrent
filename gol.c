#include <stdio.h>
#include <stdlib.h>
typedef unsigned char cell_t;


cell_t** allocate_board(int size);
void free_board(cell_t** board, int size);
int adjacent_to(cell_t** board, int size, int i, int j);
void play(cell_t** board, cell_t** newboard, int size);
void print_board(cell_t** board, int size);
void read_file(FILE* f, cell_t** board, int size);


int main() {
    FILE *f;
    f = stdin;

    int size, steps;
    fscanf(f, "%d %d", &size, &steps);

    cell_t** prev = allocate_board(size);
    read_file(f, prev, size);
    fclose(f);

    cell_t** next = allocate_board(size);
    cell_t** tmp;

    #ifdef DEBUG
    printf("Initial:\n");
    print_board(prev, size);
    #endif

    for (int i = 0; i < steps; ++i) {
        play(prev, next, size);

        #ifdef DEBUG
        printf("%d ----------\n", i + 1);
        print_board(next,size);
        #endif

        tmp = next;
        next = prev;
        prev = tmp;
    }

    #ifdef RESULT
    printf("Final:\n");
    print_board(prev,size);
    #endif

    free_board(prev, size);
    free_board(next, size);
}

cell_t** allocate_board(int size) {
    cell_t** board = (cell_t**) malloc(sizeof(cell_t*) * size);
    for (int i = 0; i < size; ++i) {
        board[i] = (cell_t*) malloc(sizeof(cell_t) * size);
    }
    return board;
}

void free_board(cell_t** board, int size) {
    for (int i = 0; i < size; ++i) {
        free(board[i]);
    }
    free(board);
}

int adjacent_to(cell_t** board, int size, int i, int j) {
    int start_line = i, end_line = i;
    int start_column = j, end_column = j;

    if (i > 0) {
        --start_line;
    }

    if (i < size - 1) {
        ++end_line;
    }

    if (j > 0) {
        --start_column;
    }

    if (j < size - 1) {
        ++end_column;
    }

    int count = 0;
    for (int l = start_line; l <= end_line; ++l) {
        for (int c = start_column; c <= end_column; ++c) {
            count += board[l][c];
        }
    }

    count -= board[i][j];
    return count;
}

void play(cell_t** board, cell_t** newboard, int size) {
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            int a = adjacent_to(board, size, i, j);
            if (a < 2 || a > 3) newboard[i][j] = 0;
            if (a == 2) newboard[i][j] = board[i][j];
            if (a == 3) newboard[i][j] = 1;
        }
    }
}

void print_board(cell_t** board, int size) {
    for (int j = 0; j < size; ++j) {
        for (int i = 0; i < size; ++i) {
            printf ("%c", board[i][j] ? 'x' : ' ');
        }
        printf ("\n");
    }
}


void read_file(FILE* f, cell_t** board, int size) {
    char* s = (char*) malloc(size + 10);
    fgets(s, size + 10, f);

    for (int j = 0; j < size; ++j) {
        fgets(s, size + 10, f);
        for (int i = 0; i < size; ++i) {
            board[i][j] = (s[i] == 'x');
        }
    }
}
