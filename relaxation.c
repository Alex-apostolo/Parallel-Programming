#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
// TODO: REMOVE BEFORE UPLOADING
#include "pthread_barrier.c"

pthread_barrier_t barrier;
pthread_barrier_t barrier1;

bool success = false;
double **current;
double **previous;

typedef struct args {
    int index;
    int dimension;
    double precision;
    struct CoordNode *coordinates_first;
    struct CoordNode *coordinates_last;
} ARGS;

struct ThreadNode {
    ARGS *arg;
    struct ThreadNode *next;
};

struct Coordinates {
    int i;
    int j;
};

struct CoordNode {
    struct Coordinates tuple;
    struct CoordNode *next;
};

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
            current[i][j] = (previous[i][j - 1] + previous[i - 1][j] +
                             previous[i][j + 1] + previous[i + 1][j]) /
                            4;
            temp = temp->next;
        }

        pthread_barrier_wait(&barrier);

        // No need to lock since only the first thread is responsible for changing the success values
        if (index == 0) {
            success = true;
            for (i = 1; i < dimension - 1; i++) {
                for (j = 1; j < dimension - 1; j++) {
                    if (current[i][j] - previous[i][j] < precision)
                        success = success && true;
                    else
                        success = success && false;
                }
            }

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

    int reps;
    if (pthreads < (dimension - 2) * (dimension - 2))
        reps = pthreads;
    else
        reps = (dimension - 2) * (dimension - 2);

    // Creates a circular linked list of ThreadNodes
    struct ThreadNode *first;
    struct ThreadNode *last;
    for (int i = 0; i < reps; i++) {
        // Allocate memory for the fields of the ThreadNode
        struct ThreadNode *temp = malloc(sizeof(struct ThreadNode));
        // Initializes args
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
            // Traverse to the right of the linked list
            last->next = temp;
            last = temp;
        }
    }
    // Makes the linked list circular
    last->next = first;

    // Traverse the "current" matrix and assign the coordinates to threads in a
    // circular fashion
    struct ThreadNode *temp = first;
    for (int i = 1; i < dimension - 1; i++) {
        for (int j = 1; j < dimension - 1; j++) {
            struct Coordinates *coordinates =
                malloc(sizeof(struct Coordinates));
            coordinates->i = i;
            coordinates->j = j;

            if (temp->arg->coordinates_first == NULL) {
                temp->arg->coordinates_first = malloc(sizeof(struct CoordNode));
                temp->arg->coordinates_first->tuple = *coordinates;
                temp->arg->coordinates_last = temp->arg->coordinates_first;
            } else {
                temp->arg->coordinates_last->next =
                    malloc(sizeof(struct CoordNode));
                temp->arg->coordinates_last->next->tuple = *coordinates;
                temp->arg->coordinates_last = temp->arg->coordinates_last->next;
            }
            temp = temp->next;
        }
    }

    pthread_t tids[reps];
    pthread_barrier_init(&barrier, NULL, reps);
    pthread_barrier_init(&barrier1, NULL, reps);

    //error if it cant create it
    temp = first;
    for (int i = 0; i < reps; i++) {
        pthread_create(&tids[i], NULL, solve, temp->arg);
        temp = temp->next;
    }

    temp = first;
    for (int i = 0; i < reps; i++) {
        pthread_join(tids[i], NULL);
        temp = temp->next;
    }

    pthread_barrier_destroy(&barrier);
    pthread_barrier_destroy(&barrier1);
    printSquare(previous, dimension);
    return 0;
}

int main() {
    // Prompt User for dimension of their array
    // Call relaxation
    relaxation(7, 8, 0.002);
    return 0;
}