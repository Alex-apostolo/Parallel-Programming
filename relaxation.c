#include <stdio.h>
#include <stdlib.h>

void initSquare(double ***square, int dimension) {
    *square = malloc(dimension * sizeof(double *));
    for(int i = 0; i < dimension; i++){
        (*square)[i] = malloc(dimension * sizeof(double));
        for(int j = 0; j < dimension; j++){
            if(i == 0 || j == 0) (*square)[i][j] = 1;
        }
    }
}

void printSquare(double ***square, int dimension) {
    for(int i = 0; i < dimension; i++) {
        for(int j = 0; j < dimension; j++) {
            printf("%f\t",(*square)[i][j]);
        }
        printf("\n");
    }
}

int relaxation(double **square,int dimension,int pthreads,double precision) {
    initSquare(&square,dimension);
    printSquare(&square,dimension);
    return 0;
}

int main() {
    //Prompt User for dimension of their array
    int dimension = 4;
    //Declare square array
    double **square;
    //Call relaxation
    relaxation(square,dimension,2,0.01);
    return 0;
}