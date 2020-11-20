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
    double **current;
    double **previous;
    int dimension;
    int startrow;
    int endrow;
    int startcol;
    int endcol;
    double precision;
    int index;
} ARGS;

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
    int startrow = targs->startrow;
    int endrow = targs->endrow;
    int startcol = targs->startcol;
    int endcol = targs->endcol;
    double precision = targs->precision;
    int index = targs->index;

    while (success == false) {
        for (int i = startrow; i < endrow; i++) {
            for (int j = startcol; j < endcol; j++) {
                current[i][j] = (previous[i][j - 1] + previous[i - 1][j] +
                                 previous[i][j + 1] + previous[i + 1][j]) /
                                4;
            }
        }
        pthread_barrier_wait(&swapncheck);

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

    int calculations = (dimension - 2) * (dimension - 2);
    if (pthreads >= calculations) {
        pthread_t tids[calculations];
        ARGS args[calculations];
        pthread_barrier_init(&swapncheck, NULL, calculations);
        pthread_barrier_init(&texit, NULL, calculations);
        int c = 0;
        for (int i = 1; i < dimension - 1; i++) {
            for (int j = 1; j < dimension - 1; j++) {
                args[c].current = current;
                args[c].previous = previous;
                args[c].dimension = dimension;
                args[c].startrow = i;
                args[c].endrow = i + 1;
                args[c].startcol = j;
                args[c].endcol = j + 1;
                args[c].precision = precision;
                args[c].index = c;
                pthread_create(&tids[c], NULL, solve, &args[c]);
                c++;
            }
        }
        for (c = 0; c < calculations; c++) {
            pthread_join(tids[c], NULL);
        }
    } else if (pthreads % calculations == 0) {
    }
    printSquare(previous, dimension);
    return 0;
}

int main() {
    // Prompt User for dimension of their array
    // Call relaxation
    relaxation(3, 6, 0.0000001);
    return 0;
}