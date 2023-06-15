#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

double calculate_mean(int *arr, int size) {
    int i;
    double sum = 0.0;

#pragma omp parallel for reduction(+:sum)
    for (i = 0; i < size; i++) {
        sum += arr[i];
    }

    return sum / size;
}

double calculate_variance(int *arr, int size, double mean) {
    int i;
    double squared_diff_sum = 0.0;

#pragma omp parallel for reduction(+:squared_diff_sum)
    for (i = 0; i < size; i++) {
        double diff = arr[i] - mean;
        squared_diff_sum += diff * diff;
    }

    return squared_diff_sum / size;
}

int main() {
    int size, i;
    double mean, variance, std_deviation;
    int num_threads;

    printf("Array size: ");
    scanf("%d", &size);

    int arr[size];

    // Generate random array elements
    srand(time(NULL));
    for (i = 0; i < size; i++) {
        arr[i] = rand() % 100; // Generate random numbers between 0 and 99
    }

    printf("Number of threads: ");
    scanf("%d", &num_threads);

    omp_set_num_threads(num_threads);

    double start_time = omp_get_wtime();

    double total_sum = 0.0;
    int remainder = size % num_threads;

#pragma omp parallel shared(total_sum)
    {
        double local_sum = 0.0;
        int thread_count = omp_get_num_threads();
        int thread_id = omp_get_thread_num();
        int chunk_size = size / thread_count;
        int start = thread_id * chunk_size;
        int end = start + chunk_size;

        if (thread_id == thread_count - 1) {
            end += remainder;
        }

        for (i = start; i < end; i++) {
            local_sum += arr[i];
        }

#pragma omp critical
        {
            total_sum += local_sum;
        }
    }

    mean = total_sum / size;

    double total_squared_diff_sum = 0.0;

#pragma omp parallel shared(total_squared_diff_sum)
    {
        double local_squared_diff_sum = 0.0;
        int thread_count = omp_get_num_threads();
        int thread_id = omp_get_thread_num();
        int chunk_size = size / thread_count;
        int start = thread_id * chunk_size;
        int end = start + chunk_size;

        if (thread_id == thread_count - 1) {
            end += remainder;
        }

        for (i = start; i < end; i++) {
            double diff = arr[i] - mean;
            local_squared_diff_sum += diff * diff;
        }

#pragma omp critical
        {
            total_squared_diff_sum += local_squared_diff_sum;
        }
    }

    variance = total_squared_diff_sum / size;
    std_deviation = sqrt(variance);

    double end_time = omp_get_wtime();
    double parallel_time = end_time - start_time;

    printf("Mean = %.2lf, Variance = %.4lf, Standard deviation = %.4lf\n", mean, variance, std_deviation);
    printf("Parallel Time: %.6lf seconds\n", parallel_time);

    return 0;
}