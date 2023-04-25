#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>

#define DEBUG 1

/* Translation of the DNA bases
   A -> 0
   C -> 1
   G -> 2
   T -> 3
   N -> 4*/

#define M  1000000 // Number of sequences
#define N  200  // Number of bases per sequence

unsigned int g_seed = 0;

int fast_rand(void) {
    g_seed = (214013*g_seed+2531011);
    return (g_seed>>16) % 5;
}

// The distance between two bases
int base_distance(int base1, int base2){

    if((base1 == 4) || (base2 == 4)){
        return 3;
    }

    if(base1 == base2) {
        return 0;
    }

    if((base1 == 0) && (base2 == 3)) {
        return 1;
    }

    if((base2 == 0) && (base1 == 3)) {
        return 1;
    }

    if((base1 == 1) && (base2 == 2)) {
        return 1;
    }

    if((base2 == 2) && (base1 == 1)) {
        return 1;
    }
    return 2;
}

int main(int argc, char *argv[] ) {

    int i, j, numprocs, rank, checksum_extra;
    int *data1, *data2, *d1_scatterBuff, *d2_scatterBuff, *result;
    struct timeval  tv1, tv2;
    int rows = M/numprocs;

    //Inicializacion de MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    /* Initialize Matrices */
    if(rank == 0){
        data1 = (int *) malloc(M*N*sizeof(int));
        data2 = (int *) malloc(M*N*sizeof(int));
      
        for(i=0;i<M;i++) {
            for(j=0;j<N;j++) {
            /* random with 20% gap proportion */
            data1[i*N+j] = fast_rand();
            data2[i*N+j] = fast_rand();
            }
        }
    }

    d1_scatterBuff = (int *) malloc((rows*N)*sizeof(int));
    d2_scatterBuff = (int *) malloc((rows*N)sizeof(int));
    result_scatter = (int *) malloc((rows)*sizeof(int));

    gettimeofday(&tv1, NULL);

    MPI_Scatter(data1, rows, MPI_INT, d1_scatterBuff, rows, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(data2, rows, MPI_INT, d2_scatterBuff, rows, MPI_INT, 0, MPI_COMM_WORLD);

    gettimeofday(&tv2, NULL);

    int microseconds = (tv2.tv_usec - tv1.tv_usec) + 1000000 * (tv2.tv_sec - tv1.tv_sec);

    gettimeofday(&tv1, NULL);

    for(i = 0; i < rows; i++){
        result_scatter[i] = 0;
        for(j=0;j<N;j++) {
              result_scatter[i] += base_distance(d1_scatterBuff[i*N+j], d2_scatterBuff[i*N+j]);
        } 
    }

    if(rank == 0){
        for(i = rows*numprocs; i < M; i++){
            for(j=0;j<N;j++) {
                result += base_distance(data1[i*N+j], data2[i*N+j]);
            }
        }
    }

    gettimeofday(&tv2, NULL);
      
    int microseconds2 = (tv2.tv_usec - tv1.tv_usec)+ 1000000 * (tv2.tv_sec - tv1.tv_sec);

    /* Display result */
    if (DEBUG == 1) {
        int checksum = 0;

        gettimeofday(&tv1, NULL);

        MPI_Gather(result_scatter, rows, MPI_INT, &checksum, rows, MPI_INT, 0, MPI_COMM_WORLD);
        
        gettimeofday(&tv2, NULL);

        int microseconds += (tv2.tv_usec - tv1.tv_usec) + 1000000 * (tv2.tv_sec - tv1.tv_sec);

        if(rank == 0){
          printf("Checksum: %d\n ", checksum);
        }
    } else if (DEBUG == 2) {
        for(i = 0; i < rows; i++){
          printf(" %d \t ",result_scatter[i]);
        }
    } else {
        printf ("Time (seconds) of communication = %lf\n", (double) microseconds/1E6);
        printf ("Time (seconds) of computing = %lf\n", (double) microseconds2/1E6);
    }    

    free(data1); free(data2); free(result_scatter); free(d1_scatterBuff); free(d2_scatterBuff);

    MPI_Finalize();

    return 0;
}