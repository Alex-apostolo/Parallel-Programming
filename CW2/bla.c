#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

void parallelFunc(double **, double **, int, float, int, int);

int main(int argc, char **argv)
{
    double *current;
    double *previous;
    parallelFunc(&previous, &current, 5, 0.01, 1, 0);
}

/* Creates array with size dimension^2 with padded edges */
void initSquare(double **square, int dimension) {
    int size = dimension * dimension;
    *square = calloc(1, size * sizeof(double));
    int i;
    // Tracks first row
    int a;
    // Tracks last row
    int b;
    // Tracks first column
    int c;
    // Tracks last column
    int d;

    // Fills first row, column and last row, column of array with 1s
    for (i = 0, a = 0, b = size - dimension, c = 0, d = dimension - 1;
         i < dimension; i++, a++, b++, c += dimension, d += dimension) {
        (*square)[a] = 1;
        (*square)[b] = 1;
        (*square)[c] = 1;
        (*square)[d] = 1;
    }
}

/* Prints array given the dimension */
void printSquare(double *square, int dimension) {
    int i;
    int j;
    // k keeps track of last index element
    int k;
    for (i = 0, k = 0; i < dimension; i++) {
        for (j = 0; j < dimension; j++, k++) {
            printf("%f\t", square[k]);
        }
        printf("\n");
    }
    printf("\n");
}

/* Parallel function responsible for calculating the subpart of the array
 * depending on the rank And combining everything into the current array of the
 * first rank
 */
void parallelFunc(double **previous, double **current, int dimension,
                  float precision, int processors, int rank) {
    // to find the number of calculations this processor will do
    // take the size and divide it by the number of processors
    int calculations = (dimension * dimension) / processors;
    // calculate which range of values this processor will compute
    int x0 = rank * calculations;
    int x1 = (rank + 1) * calculations;

    // If previous is NULL then initialize it
    if (*previous == NULL) {
        // Only initialize the values needed for the computation of current 
        // Create an array padded with 1s on the edges
        initSquare(previous, dimension);
    }
    // If current is NULL then initialize it
    if (*current == NULL) {
        *current = malloc((x1 - x0) * sizeof(double));
    }

    int i;
    int k;
    for (i = x0, k = 0; i < x1; i++, k++) {
        if ((*previous)[i] != 1) {
            (*current)[k] =
                ((*previous)[i - 1] + (*previous)[i + 1] +
                 (*previous)[i - dimension] + (*previous)[i + dimension]) /
                4;
        } else if ((*previous)[i] == 1 && (*current)[k] != 1) {
            (*current)[k] = 1;
        }
    }

    printSquare(*current, dimension);

    if (rank != 0) {
        // Send messages to processor 0
        // int ierr = MPI_Send();
    }
    // Barrier responsible for waiting for all the processors to finish
    // their execution. It is placed right before the combination of the sub
    // arrays
    // int ierr = MPI_Barrier(MPI_COMM_WORLD);

    // When every processor is done with their computations combine into one
    // array. The first processor receives messages from the rest of the
    // processors which contain their calculated parts of the array
    if (rank == 0) {
        // Receive messages and combine arrays to a single one
    }
}

long double sumSquare(double **square, int dimension) {
    int i;
    int j;
    long double sum = 0.0;
    for (i = 0; i < dimension; i++) {
        for (j = 0; j < dimension; j++) {
            sum = sum + square[i][j];
        }
    }
    return sum;
}

// /*This method is called when a thread gets created*/
// void *thread_func(void *args) {
//     // Casts void pointer to ARGS pointer
//     ARGS *targs = args;
//     // Puts the fields of the args struct to seperate variables for easier use
//     int index = targs->index;
//     int dimension = targs->dimension;
//     double precision = targs->precision;
//     struct CoordNode *coordinates_first = targs->coordinates_first;
//     // struct CoordNode *coordinates_last = targs->coordinates_last;

//     while (success == false) {
//         struct CoordNode *temp = coordinates_first;
//         int i, j;
//         while (temp != NULL) {
//             i = temp->tuple.i;
//             j = temp->tuple.j;
//             // No need to lock here since each thread changes different values
//             // inside the array
//             current[i][j] = (previous[i][j - 1] + previous[i - 1][j] +
//                              previous[i][j + 1] + previous[i + 1][j]) /
//                             4;
//             temp = temp->next;
//         }

//         pthread_barrier_wait(&barrier);

//         // No need to lock here either since only the first thread is
//         // responsible for changing the success values
//         if (index == 0) {
//             // Checks if each value of the calculated array is below precision
//             success = true;
//             for (i = 1; i < dimension - 1; i++) {
//                 for (j = 1; j < dimension - 1; j++) {
//                     if (current[i][j] - previous[i][j] < precision)
//                         success = success && true;
//                     else
//                         success = success && false;
//                 }
//             }

//             // Swap previous and current
//             double **temp1 = previous;
//             previous = current;
//             current = temp1;
//         }

//         pthread_barrier_wait(&barrier1);
//     }
//     pthread_exit(NULL);
// }