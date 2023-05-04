#include <stdio.h>
#include <mpi/mpi.h>
/*
 * Roaa Gamal Yousef 20210571
 * Nada Shaben Omar 20210611
 * */
int main(int argc, char *argv[]) {
    int num_steps = 1000000;
    double step, x, sum = 0.0, pi;
    int i, rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // calculate the step value and broadcast it to all processes
    step = 1.0 / (double) num_steps;
    MPI_Bcast(&step, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // calculate the partial sum for each process
    int start = (rank * num_steps) / size;
    int end = ((rank + 1) * num_steps) / size;
    for (i = start; i < end; i++) {
        x = ((double)(i + 0.5)) * step;
        sum += 4.0 / (1.0 + x * x);
    }

    // use MPI_Reduce to combine the partial sums and calculate the final pi value
    MPI_Reduce(&sum, &pi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    // output the pi value from the master process
    if (rank == 0) {
        pi *= step;
        printf("%.20f\n", pi);
    }

    MPI_Finalize();
    return 0;
}
