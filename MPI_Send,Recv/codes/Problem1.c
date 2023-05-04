#include <stdio.h>
#include <stdlib.h>
#include "mpi/mpi.h"

int main(int argc, char *argv[]) {
    int size, rank;
    int n, *arr, *recvbuf;
    int i, max, max_index, local_max, local_max_index;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        printf("Hello from master process.\n");
        printf("Number of slave processes is %d\n", size - 1);

        printf("Please enter size of array...\n");
        scanf("%d", &n);

        arr = (int *) malloc(n * sizeof(int));
        printf("Please enter array elements...\n");
        for (i = 0; i < n; i++) {
            scanf("%d", &arr[i]);
        }

        // send the size of the array to all processes
        for (i = 1; i < size; i++) {
            MPI_Send(&n, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }

        // send the partition of the array to each process
        for (i = 1; i < size; i++) {
            int start = (i - 1) * (n / (size - 1));
            int end = (i == size - 1) ? n : i * (n / (size - 1));
            int partition_size = end - start;
            MPI_Send(&arr[start], partition_size, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    } else {
        // receive the size of the array
        MPI_Recv(&n, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // receive the partition of the array
        int start = (rank - 1) * (n / (size - 1));
        int end = (rank == size - 1) ? n : rank * (n / (size - 1));
        int partition_size = end - start;
        recvbuf = (int *) malloc(partition_size * sizeof(int));
        MPI_Recv(recvbuf, partition_size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // calculate the max and its index in the partition
        local_max = recvbuf[0];
        local_max_index = 0;
        for (i = 1; i < partition_size; i++) {
            if (recvbuf[i] > local_max) {
                local_max = recvbuf[i];
                local_max_index =  i;
            }
        }

        // send the local max and its index back to the master process
        MPI_Send(&local_max, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(&local_max_index, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

        free(recvbuf);
    }

    if (rank == 0) {
        max = arr[0];
        max_index = 0;
        for (i = 1; i < size; i++) {
            int slave_max, slave_max_index;
            MPI_Recv(&slave_max, 1,MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&slave_max_index, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (slave_max > max) {
                max = slave_max;
                max_index = slave_max_index + (i - 1) * (n / (size - 1));
            }
            printf("Hello from slave#%d Max number in my partition is %d and index is %d.\n", i, slave_max, slave_max_index);
        }
        printf("Master process announce the final max which is %d and its index is %d.\n", max, max_index);
    } else{
        // receive the size of the array
        MPI_Recv(&n, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // receive the partition of the array
        int start = (rank - 1) * (n / (size - 1));
        int end = (rank == size - 1) ? n : rank * (n / (size - 1));
        int partition_size = end - start;
        recvbuf = (int *) malloc(partition_size * sizeof(int));
        MPI_Recv(recvbuf, partition_size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);


        // send the local max and its index back to the master process
        MPI_Send(&local_max, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(&local_max_index, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

        free(recvbuf);
    }
    MPI_Finalize();
    return 0;
}