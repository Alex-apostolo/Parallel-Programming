#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Initializes sub_array for the ranges x0 to x1 of the actual array */
void init_sub_array(double **sub_array, int dimension, int x0, int x1) {
    // Total size of array
    int size = dimension * dimension;
    // Initialize sub_array with zeroes
    *sub_array = calloc(1, (x1 - x0) * sizeof(double));

    // Fill sub_array
    int i;
    int k;
    for (i = x0, k = 0; i < x1; i++, k++) {
        // If first row
        if((i >= 0) && (i < dimension))
            (*sub_array)[k] = 1;
        // If last row
        if((i >= (size - dimension)) && (i < size))
            (*sub_array)[k] = 1;
        // If first column
        if((i % dimension) == 0)
            (*sub_array)[k] = 1;
        // // If last column
        if((i % dimension) == (dimension - 1))
            (*sub_array)[k] = 1;
    }
}

/* Prints array given the dimension */
void print_array(double *array, int dimension, int x0, int x1) {
    for(int i = 1; i <= (x1-x0); i++) {
        printf("%f\t", array[i-1]);
        if((i % dimension) == 0)
            printf("\n");
    }
}

void solver(double **previous, double **current, int dimension, double precision,
            int processors, int rank) {
    // to find the number of calculations this processor will do
    // take the size and divide it by the number of processors
    int calculations = (dimension * dimension) / processors;
    // calculate which range of values this processor will compute
    int x0 = rank * dimension;
    int x1 = x0 + (3 * dimension);
    printf("x0: %d, x1: %d\n", x0, x1);
    //init_sub_array(previous, dimension, 4, 16);
    // Initialize previous and current 
    if(*previous == NULL) {
        init_sub_array(previous, dimension, x0, x1);
    }
    if(*current == NULL) {
        *current = malloc((x1 - x0)*sizeof(double));
    }
    print_array(*previous, dimension, x0, x1);
}

int main(int argc, char *argv[]) {
    // // For debugging
    // argc = 3;
    // argv[1] = argv[4];
    // argv[2] = argv[5];
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