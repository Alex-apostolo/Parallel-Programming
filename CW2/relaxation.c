#include "pthread_barrier.c"
#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
void solver(double **previous, double **current, int dimension,
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
    // Iterate through range and apply algorithm to previous
    // Result is stored in currents k index
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
    int ierr = MPI_Barrier(MPI_COMM_WORLD);

    // When every processor is done with their computations combine into one
    // array. The first processor receives messages from the rest of the
    // processors which contain their calculated parts of the array
    if (rank == 0) {
        // Receive messages and combine arrays to a single one
    } 
}

int main(int argc, char *argv[]) {
    // Basic handling of CLI input
    if (argc != 3) {
        fprintf(stderr, "Error: need to pass 2 arguments - the dimension "
                        "and the precision. In that precise order!\n");
        return -1;
    }
    // Dimension is the number of rows or columns, for example dimension = 5
    // will make an 2D array of 5 x 5
    int dimension = atoi(argv[1]);
    double precision = atof(argv[2]);
    if (dimension <= 0 || precision <= 0.0) {
        fprintf(stderr, "Error: make sure you entered correct values for "
                        "dimension, threads and precision\n");
        return -1;
    }

    // Initialization of the MPI environment
    int ierr = MPI_Init(NULL, NULL);

    // Get number of processors associated with the communicator
    int mpi_size;
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

    // Get rank of the calling processor
    int mpi_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);


    // if (pthreads != 1) {
    //    printf("%d,%d,%f,", dimension, pthreads, precision);
    //}
    // Commented out lines where used for testing
    // struct timespec start, finish;
    // double elapsed;

    // clock_gettime(CLOCK_MONOTONIC, &start);

    // Runs solver, calculates current array until the
    // precision is met
    double *previous = NULL;
    double *current = NULL;
    solver(&previous, &current, dimension, precision, mpi_size, mpi_rank);

    // clock_gettime(CLOCK_MONOTONIC, &finish);

    // elapsed = (finish.tv_sec - start.tv_sec);
    // elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    // printf("%f,",elapsed);
    // // Calculate result and put it here
    //  if (pthreads != 1) {
    //     printf("%Lf,", sumSquare(previous,dimension));
    // } else {
    //     printf("%Lf\n", sumSquare(previous,dimension));
    // }

     // Finalizes MPI and after this resources can be freed
    MPI_Finalize();

    return 0;
}