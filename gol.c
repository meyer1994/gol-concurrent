#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

typedef unsigned char cell_t;

typedef struct range {
    int start_line;
    int end_line;
} range;

// Prototypes
cell_t** allocate_board(int size);
void free_board(cell_t** board, int size);
int adjacent_to(cell_t** board, int size, int i, int j);
void print_board(cell_t** board, int size);
void read_file(FILE* f, cell_t** board, int size);

sem_t sem;  // sempahore
sem_t mutex;  // mutex

// Global variables
cell_t** prev;
cell_t** next;
cell_t** tmp;
int size;
int steps;
int start_line;
int end_line;
int total;

// Do the thing
void* play(void* arg) {
    while(steps > 0) {

        sem_wait(&sem);
        sem_wait(&mutex);

        // calculate range
        int thr;
        sem_getvalue(&sem, &thr);
        printf("thr %d\n", thr);
        start_line = (thr * (size / total));
        int end = start_line + (size / total);
        if (end >= size)
            end  = size - 1;
        end_line = end;

        printf("start %d\n", start_line);
        printf("end %d\n\n", end_line);

        // get coordinates
        for (int i = start_line; i < end_line; i++) {
            for (int j = 0; j < size; j++) {
                int a = adjacent_to(prev, size, i, j);
                if (a < 2 || a > 3)
                    next[i][j] = 0;
                if (a == 2)
                    next[i][j] = prev[i][j];
                if (a == 3)
                    next[i][j] = 1;
            }
        }

        if (end_line == (size - 1)) {
            steps--;
            start_line = 0;

            // swap time
            cell_t** tmp = next;
            next = prev;
            prev = tmp;

            for (int i = 0; i < total; i++)
                sem_post(&sem);

        }
        sem_post(&mutex);

        // debug stuff
        #ifdef DEBUG
        printf("%d ----------\n", i + 1);
        print_board(next, size);
        #endif
    }

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

    // insantiate stuff
    total = atoi(argv[1]);
    start_line = 0;
    pthread_t threads[total];
    pthread_t contr;
    sem_init(&sem, 0, total);
    sem_init(&mutex, 0, 1);

    // start threads
    for (int i = 0; i < total; i++) {
        range* coord = malloc(sizeof(range));


        pthread_create(&threads[i], NULL, play, NULL);
    }

    for (int i = 0; i < total; i++)
        pthread_join(threads[i], NULL);

    sem_destroy(&sem);
    sem_destroy(&mutex);

    // more debug stuff
    // #ifdef RESULT
    // printf("Final:\n");
    // print_board(prev,size);
    // #endif

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
