#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct args {
    double **current;
    double **previous;
    int dimension;
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
    //Coerse void pointer to ARGS pointer
    ARGS *targs = args;
    double **current = targs->current;
    double **previous = targs->previous;
    int dimension = targs->dimension;
    double precision = targs->precision;

    bool success = false;
    while (success == false) {
        // Success starts of as true and if one element does not meet the
        // precision goes to false
        success = true;
        for (int i = 1; i < dimension - 1; i++) {
            for (int j = 1; j < dimension - 1; j++) {
                current[i][j] = (previous[i][j - 1] + previous[i - 1][j] +
                                 previous[i][j + 1] + previous[i + 1][j]) /
                                4;
                if (current[i][j] - previous[i][j] <= precision)
                    success = success && true;
                else
                    success = success && false;
            }
        }
        double **temp = previous;
        previous = current;
        current = temp;
    }
    return NULL;
}

int relaxation(int dimension, int pthreads, double precision) {
    //Initialize the two arrays
    double **current;
    double **previous;
    initSquare(&current, dimension);
    initSquare(&previous, dimension);

    //Thread Creation
    pthread_t tid;
    ARGS args = {current, previous, dimension, precision};
    pthread_create(&tid, NULL, solve, &args);
    pthread_join(tid,NULL);

    //printSquare(previous, dimension);
    return 0;
}

int main() {
    // Prompt User for dimension of their array
    int dimension = 150;
    // Call relaxation
    relaxation(dimension, 2, 0.000001);
    return 0;
}