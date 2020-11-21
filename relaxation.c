#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
// TODO: REMOVE BEFORE UPLOADING
#include "pthread_barrier.c"

pthread_barrier_t swapncheck;
pthread_barrier_t texit;
bool success = false;

typedef struct args {
    int index;
    double **current;
    double **previous;
    int dimension;
    double precision;
    struct CoordNode *coordinates_first;
    struct CoordNode *coordinates_last;
} ARGS;

struct Node {
    pthread_t pid;
    ARGS *arg;
    struct Node *next;
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
    // Cast void pointer to ARGS pointer
    ARGS *targs = args;
    // Put the fields of the args struct to seperate variables for easier use
    double **current = targs->current;
    double **previous = targs->previous;
    int dimension = targs->dimension;
    // int startrow = targs->startrow;
    // int endrow = targs->endrow;
    // int startcol = targs->startcol;
    // int endcol = targs->endcol;
    double precision = targs->precision;
    int index = targs->index;

    while (success == false) {
        // for (int i = startrow; i < endrow; i++) {
        //     for (int j = startcol; j < endcol; j++) {
        //         current[i][j] = (previous[i][j - 1] + previous[i - 1][j] +
        //                          previous[i][j + 1] + previous[i + 1][j]) /
        //                         4;
        //     }
        // }
        // pthread_barrier_wait(&swapncheck);

        if (index == 0) {
            success = true;
            for (int i = 1; i < dimension - 1; i++) {
                for (int j = 1; j < dimension - 1; j++) {
                    if (current[i][j] - previous[i][j] < precision)
                        success = success && true;
                    else
                        success = success && false;
                }
            }
        }
        double **temp = previous;
        previous = current;
        current = temp;

        pthread_barrier_wait(&texit);
    }
    pthread_exit(NULL);
}

int relaxation(int dimension, int pthreads, double precision) {
    // Initialize the two arrays
    double **current;
    double **previous;
    initSquare(&current, dimension);
    initSquare(&previous, dimension);

    struct Node *last;
    int calculations = (dimension - 2) * (dimension - 2);
    if (pthreads < calculations) {
        // Initialize first of linked list
        struct Node *first = malloc(sizeof(struct Node));
        pthread_t pid;
        first->pid = pid;
        ARGS *arg = malloc(sizeof(ARGS));
        arg->current = current;
        arg->previous = previous;
        arg->dimension = dimension;
        arg->precision = precision;
        arg->index = 0;
        arg->coordinates_first = NULL;
        first->arg = arg;
        first->next = NULL;

        last = first;
        // Initialize remaining elements
        for (int i = 1; i < pthreads; i++) {
            struct Node *temp = malloc(sizeof(struct Node));
            pthread_t pid;
            temp->pid = pid;
            // Initialize args in here
            ARGS *arg = malloc(sizeof(ARGS));
            arg->current = current;
            arg->previous = previous;
            arg->dimension = dimension;
            arg->precision = precision;
            arg->index = i;
            arg->coordinates_first = NULL;
            temp->arg = arg;
            temp->next = NULL;

            last->next = temp;
            last = temp;
        }
        last->next = first;

        struct Node *temp = first;
        for (int i = 1; i < dimension - 1; i++) {
            for (int j = 1; j < dimension - 1; j++) {
                struct Coordinates *coordinates =
                    malloc(sizeof(struct Coordinates));
                coordinates->i = i;
                coordinates->j = j;

                if (temp->arg->coordinates_first == NULL) {
                    temp->arg->coordinates_first =
                        malloc(sizeof(struct CoordNode));
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

    }
    printSquare(previous, dimension);
    return 0;
}

int main() {
    // Prompt User for dimension of their array
    // Call relaxation
    relaxation(5, 2, 0.01);
    return 0;
}