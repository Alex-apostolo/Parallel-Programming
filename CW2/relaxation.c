#include "pthread_barrier.c"
// #include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Creates array with size dimension^2 */
void initSquare(double **square, int dimension) {
    int size = dimension * dimension;
    *square = calloc(1, size * sizeof(double));
    int i;
    // k keeps track of index to be filled
    int k;

    // Fills first row of array with 1s
    for (i = 0, k = 0; i < dimension; i++, k++) {
        (*square)[k] = 1;
    }
    // Fills last row of array with 1s
    for (i = 0, k = size - dimension; i < dimension; i++, k++) {
        (*square)[k] = 1;
    }
    // Fills first column of array with 1s
    for (i = 0, k = 0; i < dimension; i++, k = k + dimension) {
        (*square)[k] = 1;
    }
    // Fills last column of array with 1s
    for (i = 0, k = dimension - 1; i < dimension; i++, k = k + dimension) {
        (*square)[k] = 1;
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

void parallelFunc(double **previous, double **current, int dimension,
                  float precision, int mpi_size, int mpi_rank) {
    int i;
    int j;
    int k;
    for (i = 0, k = 0; i < dimension; i++) {
        for (j = 0; j < dimension; j++, k++) {
            // if (i == 0 || j == 0 || i == dimension - 1 || j == dimension - 1)
            // (*square)[k] = 1;
        }
    }
}

/* Solver does 3 things
 (1) Initilization of current and previous arrays
 (2) Initialization of MPI and execution of parallel function
 (3) Prints the resulting array (Optionally)
 */
int solver(double *array, int dimension, int processors, double precision) {

    /* Initilization of current and previous arrays
     * $ Code below runs sequentially $
     */

    // Size of array argument
    int size = dimension * dimension;

    // If provided more processors than the size of the array then ceil it
    processors = (processors < size) ? processors : size;

    // Current and Previous are initialized
    double *current;
    double *previous;
    if (array == NULL) {
        // If there is no supplied array then create two arrays padded with 1s
        // on the edges
        initSquare(&current, dimension);
        initSquare(&previous, dimension);
    } else {
        // Otherwise create one array for the current which will be filled later
        initSquare(&current, dimension);
        previous = array;
    }

    /* Initialization of MPI and execution of parallel function
     * $ Code below runs in parallel $
     */

    // // Initialization of the MPI environment
    // MPI_Init(NULL, NULL);

    // // Get number of processors associated with the communicator
    // int mpi_size;
    // MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

    // // Get rank of the calling processor
    // int mpi_rank;
    // MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

    // // Run parallel function here
    // parallelFunc(&previous, &current, dimension, precision, mpi_size,
    // mpi_rank);

    // // Finalizes MPI and after this resources can be freed
    // MPI_Finalize();

    /* Prints the resulting array
     * $ Code below runs sequentially $
     */

    printSquare(previous, dimension);

    return 0;
}

int main(int argc, char *argv[]) {
    // Basic handling of CLI input
    if (argc != 4) {
        fprintf(stderr,
                "Error: need to pass 3 arguments - the dimension, the number "
                "\nof threads and the precision. In that precise order!\n");
        return -1;
    }
    // Dimension is the number of rows or columns, for example dimension = 5
    // will make an array of 5 x 5
    int dimension = atoi(argv[1]);
    // remove processors
    int processors = atoi(argv[2]);
    double precision = atof(argv[3]);
    if (dimension <= 0 || processors <= 0 || precision <= 0.0) {
        fprintf(stderr, "Error: make sure you entered correct values for "
                        "dimension, threads and precision\n");
        return -1;
    }
    // if (pthreads != 1) {
    //    printf("%d,%d,%f,", dimension, pthreads, precision);
    //}
    // Commented out lines where used for testing
    // struct timespec start, finish;
    // double elapsed;

    // clock_gettime(CLOCK_MONOTONIC, &start);

    solver(NULL, dimension, processors, precision);

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
    return 0;
}