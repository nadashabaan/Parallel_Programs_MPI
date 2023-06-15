#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi/mpi.h"
#include <math.h>

double calculate_mean(int* data, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i];
    }
    return (double) sum / size;
}

double calculate_variance(int* data, int size, double mean) {
    double variance = 0;
    for (int i = 0; i < size; i++) {
        variance += pow(data[i] - mean, 2);
    }
    return variance / size;
}

int main(int argc, char** argv) {
    int rank, size;
    int* data;
    int data_size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double start_time, end_time, process_time, max_process_time;

    if (rank == 0) {
        // Get array size from user
        printf("Enter the array size: ");
        scanf("%d", &data_size);

        // Allocate memory for the array
        data = malloc(data_size * sizeof(int));

        // Generate random array elements
        srand(time(NULL));
        for (int i = 0; i < data_size; i++) {
            data[i] = rand() % 100;  // Generate random numbers between 0 and 99
        }
    }

    MPI_Barrier(MPI_COMM_WORLD); // Synchronize all processes before starting the parallel section

    start_time = MPI_Wtime(); // Start measuring process time

    // Broadcast array size
    MPI_Bcast(&data_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Calculate local sizes
    int local_size = data_size / size;
    int remainder = data_size % size;
    int* recvcounts = malloc(size * sizeof(int));
    int* displs = malloc(size * sizeof(int));

    // Calculate the receive counts and displacements for each process
    for (int i = 0; i < size; i++) {
        recvcounts[i] = local_size;
        if (i < remainder) {
            recvcounts[i]++;
        }
        displs[i] = (i > 0) ? (displs[i-1] + recvcounts[i-1]) : 0;
    }

    // Allocate memory for local data
    int* local_data = malloc(recvcounts[rank] * sizeof(int));

    // Scatter the data to all processes
    MPI_Scatterv(data, recvcounts, displs, MPI_INT, local_data, recvcounts[rank], MPI_INT, 0, MPI_COMM_WORLD);

    // Calculate local sum
    int local_sum = 0;
    for (int i = 0; i < recvcounts[rank]; i++) {
        local_sum += local_data[i];
    }

    // Gather local sums to calculate the total sum
    int total_sum;
    MPI_Reduce(&local_sum, &total_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // Calculate and broadcast the mean
    double mean = 0;
    if (rank == 0) {
        mean = (double) total_sum / data_size;
    }
    MPI_Bcast(&mean, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Calculate squared differences
    double squared_diff_sum = 0;
    for (int i = 0; i < recvcounts[rank]; i++) {
        squared_diff_sum += pow(local_data[i] - mean, 2);
    }

    // Gather squared difference sums to calculate the total sum
    double total_squared_diff_sum;
    MPI_Reduce(&squared_diff_sum, &total_squared_diff_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    // Calculate variance and standard deviation
    double variance = 0;
    double std_deviation = 0;
    if (rank == 0) {
        variance = total_squared_diff_sum / data_size;
        std_deviation = sqrt(variance);
        printf("Mean = %.1f, Variance = %.4f, Standard deviation = %.4f\n", mean, variance, std_deviation);
    }

    MPI_Barrier(MPI_COMM_WORLD); // Synchronize all processes after the parallel section

    end_time = MPI_Wtime(); // Stop measuring process time
    process_time = end_time - start_time;

    MPI_Reduce(&process_time, &max_process_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Max process time: %f\n", max_process_time);
    }

    free(data);
    free(local_data);
    free(recvcounts);
    free(displs);

    MPI_Finalize();
    return 0;
}