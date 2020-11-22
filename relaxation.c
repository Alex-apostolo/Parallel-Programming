#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
// TODO: REMOVE BEFORE UPLOADING
#include "pthread_barrier.c"

/*Global Variables*/
pthread_barrier_t barrier;
pthread_barrier_t barrier1;
bool success = false;
double **current;
double **previous;

/*Definitions of Arguments that are passed to each thread*/
typedef struct args {
    int index;
    int dimension;
    double precision;
    struct CoordNode *coordinates_first;
    struct CoordNode *coordinates_last;
} ARGS;

/*Used for Linked List of Threads*/
struct ThreadNode {
    ARGS *arg;
    struct ThreadNode *next;
};

struct Coordinates {
    int i;
    int j;
};

/*Used for Linked List of Coordinates*/
struct CoordNode {
    struct Coordinates tuple;
    struct CoordNode *next;
};

/*Creates an 2D array of size "dimension*dimension"*/
void initSquare(double ***square, int dimension) {
    *square = malloc(dimension * sizeof(double *));
    for (int i = 0; i < dimension; i++) {
        (*square)[i] = malloc(dimension * sizeof(double));
        for (int j = 0; j < dimension; j++) {
            if (i == 0 || j == 0)
                (*square)[i][j] = 1;
        }
    }
}

void printSquare(double **square, int dimension) {
    for (int i = 0; i < dimension; i++) {
        for (int j = 0; j < dimension; j++) {
            printf("%f\t", square[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

/*This method is called when a thread gets created*/
void *solve(void *args) {
    // Casts void pointer to ARGS pointer
    ARGS *targs = args;
    // Puts the fields of the args struct to seperate variables for easier use
    int index = targs->index;
    int dimension = targs->dimension;
    double precision = targs->precision;
    struct CoordNode *coordinates_first = targs->coordinates_first;
    struct CoordNode *coordinates_last = targs->coordinates_last;

    while (success == false) {
        struct CoordNode *temp = coordinates_first;
        int i, j;
        while (temp != NULL) {
            i = temp->tuple.i;
            j = temp->tuple.j;
            // No need to lock here since each thread changes different values
            // inside the array
            current[i][j] = (previous[i][j - 1] + previous[i - 1][j] +
                             previous[i][j + 1] + previous[i + 1][j]) /
                            4;
            temp = temp->next;
        }

        pthread_barrier_wait(&barrier);

        // No need to lock here either since only the first thread is
        // responsible for changing the success values
        if (index == 0) {
            // Checks if each value of the calculated array is below precision
            success = true;
            for (i = 1; i < dimension - 1; i++) {
                for (j = 1; j < dimension - 1; j++) {
                    if (current[i][j] - previous[i][j] < precision)
                        success = success && true;
                    else
                        success = success && false;
                }
            }

            // Swap previous and current
            double **temp1 = previous;
            previous = current;
            current = temp1;
        }

        pthread_barrier_wait(&barrier1);
    }
    pthread_exit(NULL);
}

int relaxation(int dimension, int pthreads, double precision) {
    // Initialize the two global arrays
    initSquare(&current, dimension);
    initSquare(&previous, dimension);

    // reps is the number of threads the program would create
    int reps;
    if (pthreads < (dimension - 2) * (dimension - 2))
        reps = pthreads;
    else
        reps = (dimension - 2) * (dimension - 2);

    // Creates a circular linked list of ThreadNodes
    struct ThreadNode *first;
    struct ThreadNode *last;
    for (int i = 0; i < reps; i++) {
        struct ThreadNode *temp = malloc(sizeof(struct ThreadNode));

        ARGS *arg = malloc(sizeof(ARGS));
        arg->dimension = dimension;
        arg->precision = precision;
        arg->index = i;
        arg->coordinates_first = NULL;
        temp->arg = arg;
        temp->next = NULL;

        // first and last point to the same ThreadNode in the start
        if (i == 0) {
            first = temp;
            last = first;
        } else {
            last->next = temp;
            last = temp;
        }
    }
    last->next = first; // Makes the linked list circular

    // Traverse the "current" matrix and assign the coordinates to threads in a
    // circular fashion
    struct ThreadNode *current = first;
    for (int i = 1; i < dimension - 1; i++) {
        for (int j = 1; j < dimension - 1; j++) {
            struct Coordinates *ctemp = malloc(sizeof(struct Coordinates));
            ctemp->i = i;
            ctemp->j = j;

            // Appends "ctemp" to Linked List
            if (current->arg->coordinates_first == NULL) {
                current->arg->coordinates_first =
                    malloc(sizeof(struct CoordNode));
                current->arg->coordinates_first->tuple = *ctemp;
                current->arg->coordinates_last =
                    current->arg->coordinates_first;
            } else {
                current->arg->coordinates_last->next =
                    malloc(sizeof(struct CoordNode));
                current->arg->coordinates_last->next->tuple = *ctemp;
                current->arg->coordinates_last =
                    current->arg->coordinates_last->next;
            }
            current = current->next;
        }
    }

    pthread_t tids[reps];
    if (pthread_barrier_init(&barrier, NULL, reps) != 0) {
        fprintf(stderr, "\nbarrier init has failed\n");
        return 1;
    };
    if (pthread_barrier_init(&barrier1, NULL, reps) != 0) {
        fprintf(stderr, "\nbarrier init has failed\n");
        return 1;
    };

    int error;
    current = first;
    for (int i = 0; i < reps; i++) {
        error = pthread_create(&tids[i], NULL, solve, current->arg);
        if (error != 0) {
            fprintf(stderr, "\nThread with index: \"%d\" could not be created",
                    i);
        }
        current = current->next;
    }

    current = first;
    for (int i = 0; i < reps; i++) {
        pthread_join(tids[i], NULL);
        current = current->next;
    }

    pthread_barrier_destroy(&barrier);
    pthread_barrier_destroy(&barrier1);
    printSquare(previous, dimension);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr,
                "Error: need to pass 3 arguments - the dimension, the number "
                "\nof threads and the precision. In that precise order!\n");
        return -1;
    }
    int dimension = atoi(argv[1]);
    int pthreads = atoi(argv[2]);
    double precision = atof(argv[3]);
    if (dimension <= 0 || pthreads <= 0 || precision <= 0.0) {
        fprintf(stderr, "Error: make sure you entered correct values for "
                        "dimension, threads and precision\n");
        return -1;
    }
    relaxation(dimension, pthreads, precision);
    return 0;
}