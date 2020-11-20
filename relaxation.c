#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct args {
    double **current;
    double **previous;
    int startrow;
    int endrow;
    int startcol;
    int endcol;
    double precision;
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
    int startrow = targs->startrow;
    int endrow = targs->endrow;
    int startcol = targs->startcol;
    int endcol = targs->endcol;
    double precision = targs->precision;

    for (int i = startrow; i < endrow; i++) {
        for (int j = startcol; j < endcol; j++) {
            current[i][j] = (previous[i][j - 1] + previous[i - 1][j] +
                             previous[i][j + 1] + previous[i + 1][j]) /
                            4;
        }
    }
    // Wait for all threads to end computation so that the array current is
    // full, then check if its below the precision, if yes then exit else swap
    // and continue the same thread

    // double **temp = previous;
    //     previous = current;
    //     current = temp;
    return NULL;
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
        int c = 0;
        for (int i = 1; i < dimension - 1; i++) {
            for (int j = 1; j < dimension - 1; j++) {
                args[c].current = current;
                args[c].previous = previous;
                args[c].startrow = i;
                args[c].endrow = i + 1;
                args[c].startcol = j;
                args[c].endcol = j + 1;
                args[c].precision = precision;
                pthread_create(&tids[i], NULL, solve, &args[c]);
                c++;
            }
        }
        for (int i = 0; i < calculations; i++) {
            pthread_join(tids[i], NULL);
        }
    } else if (pthreads % calculations == 0) {
    }
    printSquare(current,dimension);
    return 0;
}

int main() {
    // Prompt User for dimension of their array
    // Call relaxation
    relaxation(4, 4, 0.000001);
    return 0;
}