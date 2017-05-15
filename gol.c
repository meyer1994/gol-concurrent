#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

typedef unsigned char cell_t;

// Prototypes
cell_t** allocate_board(int size);
void free_board(cell_t** board, int size);
int adjacent_to(cell_t** board, int size, int i, int j);
void print_board(cell_t** board, int size);
void read_file(FILE* f, cell_t** board, int size);

sem_t sem;  // sempahore
sem_t mutex1;  // mutex
sem_t mutex;  // mutex

// Global variables
cell_t** prev;
cell_t** next;
cell_t** tmp;
int size;
int steps;
int section;

int total;

// Do the thing
void* play(void* arg) {
    while(steps > 0) {

        sem_wait(&sem);

        // só falta fazer o calculo de cada seção que cada thread vai calcular
        // tem que fazer os calculos dos limites, da coluna tal até tal e da
        // linha tal até tal.
        sem_wait(&mutex);
        int limit = size;
        int max_y = section * (size % total)

        // get coordinates
        int i = sem_value / size;
        int j = sem_value % size;

        int a = adjacent_to(prev, size, i, j);
        if (a < 2 || a > 3)
            next[i][j] = 0;
        if (a == 2)
            next[i][j] = prev[i][j];
        if (a == 3)
            next[i][j] = 1;

        sem_post(&mutex);

        // debug stuff
        #ifdef DEBUG
        printf("%d ----------\n", i + 1);
        print_board(next, size);
        #endif

        // it will only pass at the end of all tests
        if (sem_value == 0) {
            sem_wait(&mutex);
            steps--;
            section = size;
            printf("%d\n", steps);
            printf("%d %d\n", i, j);
            printf("%d\n\n", size);

            // swap time
            cell_t** tmp = next;
            next = prev;
            prev = tmp;

            for (int k = 0; k < (size * size); k++)
                sem_post(&sem);
            sem_post(&mutex);
        }
    }
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
    pthread_t threads[total];
    sem_init(&sem, 0, size*size);
    sem_init(&mutex, 0, 1);
    sem_init(&mutex1, 0, 1);

    for (int i = 0; i < total; i++)
        pthread_create(&threads[i], NULL, play, NULL);

    for (int i = 0; i < total; i++)
        pthread_join(threads[i], NULL);

    sem_destroy(&sem);
    sem_destroy(&mutex);

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
