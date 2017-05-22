#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef unsigned char cell_t;

typedef struct range {
    int min_line;
    int max_line;
} range;

// Prototypes
cell_t** allocate_board(int size);
void free_board(cell_t** board, int size);
int adjacent_to(cell_t** board, int size, int i, int j);
void print_board(cell_t** board, int size);
void read_file(FILE* f, cell_t** board, int size);

pthread_barrier_t barrier;

// Global variables
cell_t** prev;
cell_t** next;

int size;
int steps;

// Do the thing
void* play(void* arg) {
    range* rang = (range*) arg;

    // steps
    while(steps > 0) {
        // lines
        for (int i = rang->min_line; i <= rang->max_line; i++) {
            // columns
            for (int j = 0; j < size; j++) {
                // game of life rules
                int a = adjacent_to(prev, size, i, j);
                if (a < 2 || a > 3)
                    next[i][j] = 0;
                if (a == 2)
                    next[i][j] = prev[i][j];
                if (a == 3)
                    next[i][j] = 1;
            }
        }


        // all threads stops here
        int b = pthread_barrier_wait(&barrier);

        // only one of the stopped thread will do the below
        if (b == PTHREAD_BARRIER_SERIAL_THREAD) {
            cell_t** tmp = next;
            next = prev;
            prev = tmp;

            steps--;

            // debug stuff
            #ifdef DEBUG
            printf("%d ----------\n", i + 1);
            print_board(next, size);
            #endif
        }

        // wait for the boards to be changed
        pthread_barrier_wait(&barrier);
    }
    // bye!
    pthread_exit(NULL);
}

int main(int argc, char const *argv[]) {
    // arg test
    if (argc < 2) {
        printf("Usage:\ngol <num_of_threads>\n");
        exit(1);
    }

    FILE *f;
    f = stdin;

    // gets info from file
    fscanf(f, "%d %d", &size, &steps);

    // gets the initial state from file
    prev = allocate_board(size);
    read_file(f, prev, size);
    fclose(f);

    next = allocate_board(size);

    // debug stuff
    #ifdef DEBUG
    printf("Initial:\n");
    print_board(prev, size);
    #endif

    ////////////////////////////////////////////////////////////////////////////
    //                                                                        //
    //                                 PROJECT PART                           //
    //                                                                        //
    ////////////////////////////////////////////////////////////////////////////

    // insantiate stuff
    int total = atoi(argv[1]);
    pthread_t threads[total];

    // define the range of each thread
    range* ranges = malloc(sizeof(range) * total);
    int x_max = size - 1;
    int x_min = size - (size / total);

    for (int i = 0; i < total; i++) {
        ranges[i].max_line = x_max;
        ranges[i].min_line = x_min;
        x_max = x_min - 1;
        x_min = x_max - (size / total);
    }

    if (ranges[total-1].min_line != 0)
        ranges[total-1].min_line = 0;

    pthread_barrier_init(&barrier, NULL, total);

    // start threads
    for (int i = 0; i < total; i++)
        pthread_create(&threads[i], NULL, play, &ranges[i]);

    for (int i = 0; i < total; i++)
        pthread_join(threads[i], NULL);

    free(ranges);

    pthread_barrier_destroy(&barrier);

    // more debug stuff
    #ifdef RESULT
    printf("Final:\n");
    print_board(prev,size);
    #endif

    // free everything
    free_board(prev, size);
    free_board(next, size);
}

// Allocate the board in the heap
cell_t** allocate_board(int size) {
    cell_t** board = (cell_t**) malloc(sizeof(cell_t*) * size);
    for (int i = 0; i < size; ++i) {
        board[i] = (cell_t*) malloc(sizeof(cell_t) * size);
    }
    return board;
}

// Frees the heap
void free_board(cell_t** board, int size) {
    for (int i = 0; i < size; ++i) {
        free(board[i]);
    }
    free(board);
}

// Gets the number of alive neighbours
int adjacent_to(cell_t** board, int size, int i, int j) {
    int start_line = i, end_line = i;
    int start_column = j, end_column = j;

    if (i > 0)
        --start_line;
    if (i < size - 1)
        ++end_line;
    if (j > 0)
        --start_column;
    if (j < size - 1)
        ++end_column;

    int count = 0;
    for (int l = start_line; l <= end_line; ++l)
        for (int c = start_column; c <= end_column; ++c)
            count += board[l][c];

    count -= board[i][j];
    return count;
}

// Prints
void print_board(cell_t** board, int size) {
    for (int j = 0; j < size; ++j) {
        for (int i = 0; i < size; ++i) {
            printf ("%c", board[i][j] ? 'x' : ' ');
        }
        printf ("\n");
    }
}

// Reads the input file
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
