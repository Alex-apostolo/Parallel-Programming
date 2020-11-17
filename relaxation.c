#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int c = 0;

void initSquare(double ***square, int dimension)
{
    *square = malloc(dimension * sizeof(double *));
    for (int i = 0; i < dimension; i++)
    {
        (*square)[i] = malloc(dimension * sizeof(double));
        for (int j = 0; j < dimension; j++)
        {
            if (i == 0 || j == 0)
                (*square)[i][j] = 1;
        }
    }
}

void printSquare(double **square, int dimension)
{
    for (int i = 0; i < dimension; i++)
    {
        for (int j = 0; j < dimension; j++)
        {
            printf("%f\t", square[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

int relaxation(double **current, double **previous, int dimension, int pthreads, double precision)
{
    bool success = false;
    while(success == false) {
        //Success starts of as true and if one element does not meet the precision goes to false
        success = true;
        for (int i = 0; i < dimension; i++){
            for (int j = 0; j < dimension; j++){
                if (i != 0 && j != 0 && i != dimension - 1 && j != dimension - 1) {
                    current[i][j] = (previous[i][j - 1] + previous[i - 1][j] + previous[i][j + 1] + previous[i + 1][j]) / 4;
                    if(current[i][j] - previous[i][j] <= precision) success = success && true;
                    else success = success && false;
                }
            }
        }
        double **temp = previous;
        previous = current;
        current = temp;
    }
    printSquare(previous,dimension);
    return 0;
}

int main()
{
    //Prompt User for dimension of their array
    int dimension = 5;
    //Declare square array
    double **current;
    double **previous;
    initSquare(&current, dimension);
    initSquare(&previous, dimension);
    //Call relaxation
    relaxation(current, previous, dimension, 2, 0.001);
    return 0;
}