#include <stdio.h>
#include <stdlib.h>
#include "mpi/mpi.h"
#include <math.h>

int is_prime(int n) {
    if (n <= 1) {
        return 0;
    }
    for (int i = 2; i <= sqrt(n); i++) {
        if (n % i == 0) {
            return 0;
        }
    }
    return 1;
}

int main(int argc, char** argv) {
    int rank, size;
    int lower_bound, upper_bound, subrange_size;
    int sub_lower_bound, sub_upper_bound, sub_count, total_count;
    double start_time, end_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 3) {
        if (rank == 0) {
            printf("Usage: %s lower_bound upper_bound\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    if (rank == 0) {
        lower_bound = atoi(argv[1]);
        upper_bound = atoi(argv[2]);
        subrange_size = (upper_bound - lower_bound) / (size - 1);
        start_time = MPI_Wtime();
    }

    MPI_Bcast(&lower_bound, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&subrange_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    sub_lower_bound = lower_bound + (rank - 1) * subrange_size + 1;
    sub_upper_bound = sub_lower_bound + subrange_size - 1;

    if (rank == size ) {
        sub_upper_bound = upper_bound;
    }

    sub_count = 0;
    for (int i = sub_lower_bound; i <= sub_upper_bound; i++) {
        sub_count += is_prime(i);
    }

    MPI_Reduce(&sub_count, &total_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        end_time = MPI_Wtime();
        printf("Total prime numbers between %d and %d: %d\n", lower_bound, upper_bound, total_count);
        printf("Execution time: %f seconds\n", end_time - start_time);
    }

    MPI_Finalize();
    return 0;
}