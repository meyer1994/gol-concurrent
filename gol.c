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
range* get_ranges(int n, int size);

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
        for (int i = rang->min_line; i < rang->max_line; i++) {
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

        // only one of the stopped threads will do the below
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

    // avoid crazy input
    int TOTAL = atoi(argv[1]);
    if (TOTAL <= 0) {
        printf("Seriously?\n");
        exit(1);
    }
    if (TOTAL > size) {
        printf("Number of threads should be equal or smaller than the size of"
        "the matrix\nSize = %5d\nThreads = %5d", size, TOTAL);
        exit(1);
    }

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
    pthread_t threads[TOTAL];

    // define the range of each thread
    range* ranges = get_ranges(TOTAL, size);

    pthread_barrier_init(&barrier, NULL, TOTAL);

    // start threads
    for (int i = 0; i < TOTAL; i++)
        pthread_create(&threads[i], NULL, play, &ranges[i]);

    // wait to finish
    for (int i = 0; i < TOTAL; i++)
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

// return pointer to array of ranges with size n
range* get_ranges(int n, int size) {
    range* ranges = malloc(sizeof(range) * n);
    for (int i = 0; i < n; i++) {
        ranges[i].max_line = 0;
        ranges[i].min_line = 0;
    }

    int count = 0;
    while (count < size) {
        ranges[count%n].max_line++;
        count++;
    }

    for (int i = 1; i < n; i++) {
        ranges[i].min_line = ranges[i-1].max_line;
        ranges[i].max_line += ranges[i-1].max_line;
    }

    return ranges;
}

// Frees the heap
void free_board(cell_t** board, int size) {
    for (int i = 0; i < size; ++i) {
        free(board[i]);
    }
    free(board);
}

// Gets the number of alive neighbours
inline int adjacent_to(cell_t** board, int size, int i, int j) {
  int count = 0;
  if (i-1 > 0 && j - 1 > 0)
    count += board[i-1][j-1];
  if (i-1 > 0)
    count += board[i-1][j];
  if (j-1 > 0)
    count += board[i][j-1];
  if (i-1 > 0 && j+1 < size)
    count += board[i-1][j+1];
  if (j+1 < size)
    count += board[i][j+1];
  if (j-1 > 0 && i+1 < size)
    count += board[i+1][j-1];
  if (i+1 < size)
    count += board[i+1][j];
  if (i+1 < size && j+1 < size)
    count += board[i+1][j+1];
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
