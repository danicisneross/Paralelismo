#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>

#define DEBUG 0

/* Translation of the DNA bases
   A -> 0
   C -> 1
   G -> 2
   T -> 3
   N -> 4*/

#define M  10 // Number of sequences
#define N  200  // Number of bases per sequence

unsigned int g_seed = 0;

/******************
FUNCTION: fast_rand
*******************
    Genera un numero pseudoaleatortio.
*/
int fast_rand(void) {
    g_seed = (214013*g_seed+2531011);
    return (g_seed>>16) % 5;
}

/**********************
FUNCTION: base_distance
***********************
    The distance between two bases.
*/
int base_distance(int base1, int base2){

    if((base1 == 4) || (base2 == 4)){       //si una de las bases es N (desconocida)
        return 3;
    }

    if(base1 == base2) {       //si son iguales
        return 0;
    }

    if((base1 == 0) && (base2 == 3)) {      //son diferentes pero solo difieren en una letra
        return 1;
    }

    if((base2 == 0) && (base1 == 3)) {      //son diferentes pero solo difieren en una letra
        return 1;
    }

    if((base1 == 1) && (base2 == 2)) {      //son diferentes pero solo difieren en una letra
        return 1;
    }

    if((base2 == 2) && (base1 == 1)) {      //son diferentes pero solo difieren en una letra
        return 1;
    }
    return 2;       //son diferentes y difieren por mas de una letra
}

int main(int argc, char *argv[] ) {

    int i, j, numprocs, rank;
    int *data1, *data2, *d1_scatterBuff, *d2_scatterBuff, *result_scatter;
    int *result; //esto almacena todo, de aqui se saca checksum
    int comp_mean = 0, comm_mean = 0; //la media de los tiempos
    struct timeval  tv1, tv2;

    //Inicializacion de MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);   //obtenemos el numero de procesos
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);   //obtenemos el identificador de cada proceso

    int rows = M/numprocs;
    /* Initialize Matrices */
    if(rank == 0){
        result = (int *) malloc(M*sizeof(int)); 
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
    d2_scatterBuff = (int *) malloc((rows*N)*sizeof(int));
    result_scatter = (int *) malloc((rows)*sizeof(int));

    gettimeofday(&tv1, NULL);

    /* MPI_Scatter (void *buff,         int sendcnt,            MPI_Datatype sendtype,      void *recvbuff,      int recvcnt,         MPI_Datatype recvtype,        int root,        MPI_Comm comm); 
                      datos a       numero de elementos         tipo de dato a enviar.      dir del buff de   numero de elementos        tipo de dato que       proceso que manda 
                      enviar.       a enviar a cada proceso.                                   recepcion.         en el buff.               se recibe.              los datos.
    */

    MPI_Scatter(data1, rows*N, MPI_INT, d1_scatterBuff, rows*N, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(data2, rows*N, MPI_INT, d2_scatterBuff, rows*N, MPI_INT, 0, MPI_COMM_WORLD);

    gettimeofday(&tv2, NULL);

    int microseconds = (tv2.tv_usec - tv1.tv_usec) + 1000000 * (tv2.tv_sec - tv1.tv_sec);

    gettimeofday(&tv1, NULL);

    for(i = 0; i < rows; i++){
        result_scatter[i] = 0;
        for(j=0;j<N;j++) {
              result_scatter[i] += base_distance(d1_scatterBuff[i*N+j], d2_scatterBuff[i*N+j]);
        } 
    }

    //Calculo de las filas restantes que no fueron asignadas a ningun proceso
    if(rank == 0){
        for(i = rows*numprocs; i < M; i++){
            result[i] = 0;
            for(j=0;j<N;j++) {
                result[i] += base_distance(data1[i*N+j], data2[i*N+j]);
            }
        }
    }

    gettimeofday(&tv2, NULL);
      
    int microseconds2 = (tv2.tv_usec - tv1.tv_usec)+ 1000000 * (tv2.tv_sec - tv1.tv_sec);

    gettimeofday(&tv1, NULL);

    /*MPI_Gather ( void *buff,           int sendcnt,       MPI_Datatype sendtype,   void *recvbuff,        int recvcnt,         MPI_Datatype recvtype,     int root,       MPI_Comm comm) ;
                  dir de inicio        num de elementos     tipo de dato a enviar.   dir del buff de      num de elementos         tipo de dato que      proceso que hace
                del bufer de envio        que recibe                                    recepcion.    para cualquier recepcion       se recibe             la recepcion
    */

    //concatenamos los resultados individuales
    MPI_Gather(result_scatter, rows, MPI_INT, result, rows, MPI_INT, 0, MPI_COMM_WORLD);
        
    gettimeofday(&tv2, NULL);

    microseconds += (tv2.tv_usec - tv1.tv_usec) + 1000000 * (tv2.tv_sec - tv1.tv_sec);

    /* MPI_Reduce (void *buff, void *recvbuff, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm) */

    //Calculamos la media de los diferentes tiempos
    MPI_Reduce(&microseconds, &comm_mean, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&microseconds2, &comp_mean, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);


    /* Display result */
    if (rank == 0){
        if (DEBUG == 1) {
            int checksum = 0;

            for(i=0;i<M;i++) {
                checksum += result[i];
            }  
            printf(" Parallel checksum: %d\n ", checksum);

        } else if (DEBUG == 2) {
            for(i = 0; i < M*N; i++){
                printf("Parallel: %d \t \n",result[i]);
            }
        } else {
            comm_mean = comm_mean / numprocs;
            comp_mean = comp_mean / numprocs;
            printf ("Parallel with process %d: Time (seconds) of communication = %lf\n", numprocs, (double) comm_mean/1E6);
            printf ("Parallel with process %d: Time (seconds) of computing = %lf\n",  numprocs, (double) comp_mean/1E6);
        }    

        free(data1); free(data2); free(result);
    }    
        
    free(result_scatter); free(d1_scatterBuff); free(d2_scatterBuff);

    MPI_Finalize();

    return 0;
}