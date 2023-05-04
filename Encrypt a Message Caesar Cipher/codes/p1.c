#include <stdio.h>
#include <mpi/mpi.h>
#define MSG_SIZE 1000000
/*
 * Roaa Gamal Yousef 20210571
 * Nada Shaben Omar 20210611
 * */
int main(int argc, char** argv) {
    int rank,size, i, j,m, key, count, remaining=0,len;
    char input[10000];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        FILE* fp;
        fp = fopen("input.txt", "r");
        fseek(fp, 0, SEEK_END);
        len = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        fread(input, sizeof(char), len, fp);
        fclose(fp);
        printf("Enter key/shift value: ");
        scanf("%d", &key);
    }
   // printf("%s",input);

    MPI_Bcast(&key, 1, MPI_INT, 0, MPI_COMM_WORLD);
    char  encrypted[MSG_SIZE], portion[MSG_SIZE], Cipher[MSG_SIZE] ;

    MPI_Scatter(&input, MSG_SIZE / size, MPI_CHAR, &portion, MSG_SIZE / size, MPI_CHAR, 0, MPI_COMM_WORLD);
    if (rank >= 0)
    {
        for (i = 0; i < (len / size); i++)
        {
            int x = portion[i];
            if (x >= 65 && x <= 90)
            {
                char c;
                c = (((x - 65) + key) % 26) + 65;
                Cipher[i] = c;
            }
            else if(x >= 97 && x <= 122) {
                char c;
                c = (((x - 97) + key) % 26) + 97;
                Cipher[i] = c;

            }
            else
            {
                Cipher[i] = portion[i];
            }
        }
    }
    MPI_Gather(&Cipher, MSG_SIZE / size, MPI_CHAR, &encrypted, MSG_SIZE / size, MPI_CHAR, 0, MPI_COMM_WORLD);
    if (rank == 0)
    {
        if (m != len)
        {
            for (i = m; i < len; i++)
            {
                int x = input[i];
                if (x >= 65 && x <= 90)
                {
                    char c;
                    c = (((x - 65) + key) % 26) + 65;
                    encrypted[i] = c;
                }
                else if (x >= 97 && x <= 122) {
                    char c;
                    c = (((x - 97) + key) % 26) + 97;
                    encrypted[i] = c;

                }
                else
                {
                    encrypted[i] = input[i];
                }
            }
        }
        FILE* fp;
        fp = fopen("output.txt", "w");
        for (i = 0; i < len; i++)
        {
            fprintf(fp, "%c", encrypted[i]);
        }
        fclose(fp);
    }

    MPI_Finalize();
    return 0;
}
