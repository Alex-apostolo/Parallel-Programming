#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
        if ((i >= 0) && (i < dimension))
            (*sub_array)[k] = 1;
        // // If last row
        // if ((i >= (size - dimension)) && (i < size))
        //     (*sub_array)[k] = 1;
        // If first column
        if ((i % dimension) == 0)
            (*sub_array)[k] = 1;
        // // If last column
        // if ((i % dimension) == (dimension - 1))
        //     (*sub_array)[k] = 1;
    }
}

/* Prints array given the dimension */
void print_sub_array(double *sub_array, int dimension, int x0, int x1) {
    int i;
    for (i = 1; i <= (x1 - x0); i++) {
        printf("%f\t", sub_array[i - 1]);
        if ((i % dimension) == 0)
            printf("\n");
    }
}

/* Calculates the range of values the specified rank
 * will receive*/
void calculate_rank_range(int dimension, int processors, int rank, int *x0,
                          int *x1) {
    // range if only one processor
    if (processors == 1) {
        *x0 = 0;
        *x1 = dimension * dimension;
        return;
    }
    // quotient is the number of lines each processor will calculate
    int quotient = dimension / processors;
    // remainder used for last processors calculations
    int remainder = dimension % processors;
    // calculate which range of values this processor will compute
    int x0_ = rank * quotient * dimension;
    int x1_ = x0_ + (quotient * dimension);
    // Adds one extra row to be calculated by the last processor
    if (remainder != 0 && rank == processors - 1) {
        x1_ += remainder * dimension;
    }
    // Depending on the rank of the processor we add extra rows to help with the
    // computation of their values
    if (rank == 0) {
        // Adds row at the end
        x1_ += dimension;
    } else if (rank == processors - 1) {
        // Adds row at the start
        x0_ -= dimension;
    } else {
        // Adds row at the start and end
        x0_ -= dimension;
        x1_ += dimension;
    }
    *x0 = x0_;
    *x1 = x1_;
}

/* Applies the relaxation technique to the previous
 * array and storing it to current*/
int relaxation(double **previous, double **current, int size, int dimension,
               double precision) {
    // Success used to show if the current array's elements
    // are less than the precision
    int success = 1;
    int i;
    // c is used as a counter
    int c;
    // Iterate through inner array to calculate their values
    for (i = dimension + 1, c = 0; i < (size - dimension - 1); i++, c++) {
        // TEST: printf("rank: %d, i: %d c: %d\n", rank, i, c);
        if (c == dimension - 2) {
            // when we reach the last column of the inner array
            // skip two values from i and assing to c = -1
            i++;
            c = -1;
            continue;
        }
        // apply relaxation to i'th element
        (*current)[i] =
            ((*previous)[i - 1] + (*previous)[i + 1] +
             (*previous)[i - dimension] + (*previous)[i + dimension]) /
            4;

        // for the relaxation to be successful all of the computed current's
        // indexes minus the corresponding previous need to be less than the
        // precision
        success = success && (fabs((*current)[i] - (*previous)[i]) < precision);
    }
    return success;
}

/* Sends last or first row, depending on the rank, to the correct processor
 * Receives second_last or second_row, depending on the rank, to the correct
 * processor
 */
void sendrecv_row_MPI(double **current, int size, int dimension, int processors,
                      int rank, MPI_Status status) {
    double *second_row = malloc(dimension * sizeof(double));
    double *second_last_row = malloc(dimension * sizeof(double));
    int mpi_err;

    // In the case when we have only one processor, we don't need any MPI
    // communication
    if (processors == 1)
        return;

    // Send rows needed by other processors to complete their current array
    if (rank == 0) {
        // Copy second_last_row from current
        memcpy(second_last_row, &(*current)[size - (2 * dimension)],
               dimension * sizeof(double));
        // MPI_Send sends second_last_row to rank 1
        // MPI_Recv gets second_row from rank 1
        mpi_err = MPI_Sendrecv(second_last_row, dimension, MPI_DOUBLE, rank + 1,
                               0, second_row, dimension, MPI_DOUBLE, rank + 1,
                               0, MPI_COMM_WORLD, &status);
        // Checking if mpi
        if (mpi_err != MPI_SUCCESS) {
            fprintf(stderr, "Error: Failed to send or receive row\n");
            exit(-2);
        }
        // Replace last row of current with the received one
        int i;
        for (i = 0; i < dimension; i++) {
            (*current)[size - dimension + i] = second_row[i];
        }
    } else if (rank == processors - 1) {
        // Copy second_row from current
        memcpy(second_row, &(*current)[dimension], dimension * sizeof(double));
        // MPI_Send sends second_row to rank - 1
        // MPI_Recv gets second_last_row from rank - 1
        mpi_err = MPI_Sendrecv(second_row, dimension, MPI_DOUBLE, rank - 1, 0,
                               second_last_row, dimension, MPI_DOUBLE, rank - 1,
                               0, MPI_COMM_WORLD, &status);
        // Checking if mpi
        if (mpi_err != MPI_SUCCESS) {
            fprintf(stderr, "Error: Failed to send or receive row\n");
            exit(-2);
        }
        // Replace first row of current with the received one
        int i;
        for (i = 0; i < dimension; i++) {
            (*current)[i] = second_last_row[i];
        }
    } else {
        // Copy second_row from current
        memcpy(second_row, &(*current)[dimension], dimension * sizeof(double));
        // MPI_Recv gets second_last_row from rank - 1
        // MPI_Send sends second_row to rank - 1
        mpi_err = MPI_Sendrecv(second_row, dimension, MPI_DOUBLE, rank - 1, 0,
                               second_last_row, dimension, MPI_DOUBLE, rank - 1,
                               0, MPI_COMM_WORLD, &status);
        // Checking if mpi
        if (mpi_err != MPI_SUCCESS) {
            fprintf(stderr, "Error: Failed to send or receive row\n");
            exit(-2);
        }
        // Replace first row of current with the received on
        int i;
        for (i = 0; i < dimension; i++) {
            (*current)[i] = second_last_row[i];
        }

        // Copy second_last_row from current
        memcpy(second_last_row, &(*current)[size - (2 * dimension)],
               dimension * sizeof(double));
        // MPI_Send sends second_last_row to rank + 1
        // MPI_Recv gets second_row from rank + 1
        int mpi_err2 = MPI_Sendrecv(
            second_last_row, dimension, MPI_DOUBLE, rank + 1, 0, second_row,
            dimension, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &status);
        // Checking if mpi
        if (mpi_err2 != MPI_SUCCESS) {
            fprintf(stderr, "Error: Failed to send or receive row\n");
            exit(-2);
        }

        // Replace last row of current with the received one
        for (i = 0; i < dimension; i++) {
            (*current)[size - dimension + i] = second_row[i];
        }
    }
    // Frees allocated resources
    free(second_row);
    free(second_last_row);
}

/* Does (4) things
    (1) Initializes previous and current arrays
    (2) Applies relaxation
    (3) Sends correct message to other processors
    (4) Swaps current and previous and checks if to continue relaxation
*/
void solver(double **previous, double **current, int dimension,
            double precision, int processors, int rank, MPI_Status status,
            MPI_Request request, int x0, int x1) {

    // Success_for_all shows when every processor has successfully passed
    // relaxation
    int success_for_all = 0;
    // This array is used for gathering all of the successes to processor 0
    int success_arr[processors];
    while (!success_for_all) {
        // Initialize previous and current
        if (*previous == NULL || *current == NULL) {
            // Calculate the range of the values used by the arrays
            calculate_rank_range(dimension, processors, rank, &x0, &x1);
            init_sub_array(previous, dimension, x0, x1);
            init_sub_array(current, dimension, x0, x1);
        }

        // Apply relaxation to the previous array and store to current
        int success =
            relaxation(previous, current, x1 - x0, dimension, precision);

        // Send row/s to other processor/s and receive row/s from other
        // processor/s
        sendrecv_row_MPI(current, x1 - x0, dimension, processors, rank, status);

        // Each processor sends its success value to processor 0
        int ierr = MPI_Gather(&success, 1, MPI_INT, &success_arr, 1, MPI_INT, 0,
                              MPI_COMM_WORLD);
        if (ierr != 0) {
            fprintf(stderr, "Error: MPI_Gather failed\n");
            exit(-2);
        }

        // For success_for_all to be true all the gathered success's need to be
        // true Only then may the program terminate
        if (rank == 0) {
            success_for_all = 1;
            int temp;
            int i;
            for (i = 0; i < processors; i++)
                success_for_all = success_for_all && success_arr[i];
        }

        // everyone calls bcast, data is taken from root and ends up in
        // everyone's success_for_all
        ierr = MPI_Ibcast(&success_for_all, 1, MPI_INT, 0, MPI_COMM_WORLD,
                          &request);
        // everyone waits to receive the value of success_for_all
        MPI_Wait(&request, &status);

        if (ierr != 0) {
            fprintf(stderr, "Error: MPI_Ibcast failed\n");
            exit(-2);
        }

        if (!success_for_all) {
            // Swap current array with previous array
            double *temp = *previous;
            *previous = *current;
            *current = temp;
        } else {
            // For testing purposes, executed only when the program ends
            printf("Successfully completed from rank: %d\n", rank);
            print_sub_array(*current, dimension, x0, x1);
            printf("\n");
        }
    }
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
                        "dimension or precision\n");
        return -1;
    }

    MPI_Status status;
    MPI_Request request;

    // Initialization of the MPI environment
    int ierr = MPI_Init(NULL, NULL);

    if (ierr != MPI_SUCCESS) {
        fprintf(stderr, "Error: could not initialize MPI library\n");
        return -1;
    }

    // Get number of processors associated with the communicator
    int mpi_size;
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

    // Get rank of the calling processor
    int mpi_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

    if (mpi_size > dimension) {
        if (mpi_rank == 0)
            fprintf(stderr,
                    "Error: mpi_size cannot be greater than the number of "
                    "rows of the arrays\n");
        return -1;
    }

    // Commented out lines where used for testing
    // struct timespec start, finish;
    // double elapsed;

    // clock_gettime(CLOCK_MONOTONIC, &start);

    // Runs solver, calculates current array until the
    // precision is met
    // previous and current passed as NULL, to be initialized later as well
    // as the range x0, x1
    double *previous = NULL;
    double *current = NULL;
    solver(&previous, &current, dimension, precision, mpi_size, mpi_rank,
           status, request, 0, 0);

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