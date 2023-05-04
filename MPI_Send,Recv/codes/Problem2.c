#include <stdio.h>
#include <stdlib.h>
#include "mpi/mpi.h"


int main(int argc, char **argv) {
    int rank, size;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int m, n, p, q;
    int **A, **B, **C, *a, *b, *c;

    if (rank == 0) {
        // read matrix dimensions from file or console
        int mode;
        printf("Welcome to matrix multiplication program!\n");
        printf("To read dimensions and values from file press 1\n");
        printf("To read dimensions and values from console press 2\n");
        scanf("%d", &mode);

        if (mode == 1) {
            char filename[100];
            printf("Please enter the filename: ");
            scanf("%s", filename);
            FILE *file = fopen(filename, "r");
            fscanf(file, "%d %d", &m, &n);
            A = (int **)malloc(m * sizeof(int *));
            a = (int *)malloc(m * n * sizeof(int));
            for (int i = 0; i < m; i++) {
                A[i] = &(a[i * n]);
                for (int j = 0; j < n; j++) {
                    fscanf(file, "%d", &(A[i][j]));
                }
            }
            fscanf(file, "%d %d", &p, &q);
            B = (int **)malloc(p * sizeof(int *));
            b = (int *)malloc(p * q * sizeof(int));
            for (int i = 0; i < p; i++) {
                B[i] = &(b[i * q]);
                for (int j = 0; j < q; j++) {
                    fscanf(file, "%d", &(B[i][j]));
                }
            }
            fclose(file);
        } else {
            printf("Please enter dimensions of the first matrix: ");
            scanf("%d %d", &m, &n);
            A = (int **)malloc(m * sizeof(int *));
            a = (int *)malloc(m * n * sizeof(int));
            printf("Please enter its elements:\n");
            for (int i = 0; i < m; i++) {
                A[i] = &(a[i * n]);
                for (int j = 0; j < n; j++) {
                    scanf("%d", &(A[i][j]));
                }
            }
            printf("Please enter dimensions of the second matrix: ");
            scanf("%d %d", &p, &q);
            B = (int **)malloc(p * sizeof(int *));
            b = (int *)malloc(p * q * sizeof(int));
            printf("Please enter its elements:\n");
            for (int i = 0; i < p; i++) {
                B[i] = &(b[i * q]);
                for (int j = 0; j < q; j++) {
                    scanf("%d", &(B[i][j]));
                }
            }
        }

        // allocate memory for result matrix
        C = (int **)malloc(m * sizeof(int *));
        c = (int *)malloc(m * q * sizeof(int));
        for (int i = 0; i < m; i++) {
            C[i] = &(c[i * q]);
        }

        // send matrix B to all slaves
        for (int i = 1;i < size; i++) {
            MPI_Send(&p, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&q, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(B[0], p * q, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
        // distribute matrix A to slaves
        int chunk = m / (size - 1);
        int remainder = m % (size - 1);
        int offset = 0;
        for (int i = 1; i < size; i++) {
            int count = chunk * n;
            if (i <= remainder) {
                count += n;
            }
            MPI_Send(&chunk, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&n, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(A[offset], count, MPI_INT, i, 0, MPI_COMM_WORLD);
            offset += chunk;
            if (i <= remainder) {
                offset += n;
            }
        }

// receive results from slaves
        offset = 0;
        for (int i = 1; i < size; i++) {
            int count = chunk * q;
            if (i <= remainder) {
                count += q;
            }
            MPI_Recv(&(C[offset][0]), count, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
            offset += chunk;
            if (i <= remainder) {
                offset += q;
            }
        }

// print result matrix
        printf("Result Matrix is (%dx%d):\n", m, q);
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < q; j++) {
                printf("%d ", C[i][j]);
            }
            printf("\n");
        }
    } else {
// receive matrix B from master
        MPI_Recv(&p, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&q, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        B = (int **)malloc(p * sizeof(int *));
        b = (int *)malloc(p * q * sizeof(int));
        for (int i = 0; i < p; i++) {
            B[i] = &(b[i * q]);
        }
        MPI_Recv(B[0], p * q, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        // receive matrix A from master and multiply matrices
        MPI_Recv(&m, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&n, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        A = (int **)malloc(m * sizeof(int *));
        a = (int *)malloc(m * n * sizeof(int));
        for (int i = 0; i < m; i++) {
            A[i] = &(a[i * n]);
        }
        MPI_Recv(A[0], m * n, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        C = (int **)malloc(m * sizeof(int *));
        c = (int *)malloc(m * q * sizeof(int));
        for (int i = 0; i < m; i++) {
            C[i] = &(c[i * q]);
            for (int j = 0; j< q; j++) {
                C[i][j] = 0.0;
                for (int k = 0; k < n; k++) {
                    C[i][j] += A[i][k] * B[k][j];
                }
            }
        }

// send result matrix to master
        MPI_Send(&(C[0][0]), m * q, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
    MPI_Finalize();
    free(a);
    free(A);
    free(b);
    free(B);
    free(c);
    free(C);
    return 0;
}