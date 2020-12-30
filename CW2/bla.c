#include <mpi.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);
  
    // Get the number of processes ssociated with the communicator
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the calling process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // Get the name of the processor
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    printf("Hello world from process %s with rank %d out of %d processors\n", processor_name, world_rank, world_size);

    // Finalize: Any resources allocated for MPI can be freed
    MPI_Finalize();
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

/*This method is called when a thread gets created*/
void *thread_func(void *args) {
    // Casts void pointer to ARGS pointer
    ARGS *targs = args;
    // Puts the fields of the args struct to seperate variables for easier use
    int index = targs->index;
    int dimension = targs->dimension;
    double precision = targs->precision;
    struct CoordNode *coordinates_first = targs->coordinates_first;
    // struct CoordNode *coordinates_last = targs->coordinates_last;

    while (success == false) {
        struct CoordNode *temp = coordinates_first;
        int i, j;
        while (temp != NULL) {
            i = temp->tuple.i;
            j = temp->tuple.j;
            // No need to lock here since each thread changes different values
            // inside the array
            current[i][j] = (previous[i][j - 1] + previous[i - 1][j] +
                             previous[i][j + 1] + previous[i + 1][j]) /
                            4;
            temp = temp->next;
        }

        pthread_barrier_wait(&barrier);

        // No need to lock here either since only the first thread is
        // responsible for changing the success values
        if (index == 0) {
            // Checks if each value of the calculated array is below precision
            success = true;
            for (i = 1; i < dimension - 1; i++) {
                for (j = 1; j < dimension - 1; j++) {
                    if (current[i][j] - previous[i][j] < precision)
                        success = success && true;
                    else
                        success = success && false;
                }
            }

            // Swap previous and current
            double **temp1 = previous;
            previous = current;
            current = temp1;
        }

        pthread_barrier_wait(&barrier1);
    }
    pthread_exit(NULL);
}