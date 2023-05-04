#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi/mpi.h>


int is_prime(int n) {
    if (n <= 1) {
        return 0;
    }
    int i;
    for (i = 2; i <= sqrt(n); i++) {
        if (n % i == 0) {
            return 0;
        }
    }
    return 1;
}

int main(int argc, char** argv) {
    int rank, size, i;
    int x, y, r, sub_count, total_count = 0;
    int start, end;
    double start_time, end_time;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        if (argc != 3) {
            printf("Usage: %s x y\n", argv[0]);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        // read input from command line
        x = atoi(argv[1]);
        y = atoi(argv[2]);
        // calculate the subrange size
        r = (y - x + 1) / size;
        // send x and r to each slave process
        for (i = 1; i < size; i++) {
            start = x + i * r;
            end = start + r - 1;
            if (i == size - 1) {
                // handle the case where the length of the range is not divisible by the number of processes
                end = y;
            }
            MPI_Send(&start, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&end, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
        // calculate prime numbers in master process
        start = x;
        end = start + r - 1;
        if (size == 1) {
            end = y;
        }
        start_time = MPI_Wtime(); // start timing
        for (i = start; i <= end; i++) {
            if (is_prime(i)) {
                total_count++;
            }
        }
        // receive sub-count from each slave process and add to total count
        for (i = 1; i < size; i++) {
            MPI_Recv(&sub_count, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            total_count += sub_count;
        }
        end_time = MPI_Wtime(); // end timing
        // print total count of primes between x and y and execution time
        printf("Total count of primes between %d and %d is %d\n", x, y, total_count);
        printf("Execution time = %f seconds\n", end_time - start_time);
    }
    else {
        // receive start and end from master process
        MPI_Recv(&start, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&end, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        // count prime numbers in the subrange for the slave process
        sub_count = 0;
        double start_time = MPI_Wtime();

        for (i = start; i <= end; i++) {
            if (is_prime(i)) {
                sub_count++;
            }
        }

        // record end time
        double end_time = MPI_Wtime();

        // calculate execution time for the slave process
        double execution_time = end_time - start_time;

        // send the partial count and execution time to the master process
        MPI_Send(&sub_count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(&execution_time, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
