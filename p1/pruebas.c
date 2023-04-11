#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void inicializaCadena(char *cadena, int n){
    int i;
    for(i=0; i<n/2; i++){
        cadena[i] = 'A';
    }
    for(i=n/2; i<3*n/4; i++){
        cadena[i] = 'C';
    }
    for(i=3*n/4; i<9*n/10; i++){
        cadena[i] = 'G';
    }
    for(i=9*n/10; i<n; i++){
        cadena[i] = 'T';
    }
}

int sumArray(int* array, int size){
    int i;
    int sum = 0;    
    for (i=0; i < size; i++){
        sum += array[i];    
    }
    return sum;
}

int algoSec(int argc, char** argv){
      
    int i, n, count=0;
    char *cadena;
    char L;

    n = atoi(argv[1]);
    L = *argv[2];
    
    cadena = (char *) malloc(n*sizeof(char));   //todos las van a ejecutar 
    inicializaCadena(cadena, n);                //por esta linea esta en plural la de arriba
    
    for(i=0; i<n; i++){
        if(cadena[i] == L){
          count++;
        }
    }
    
    printf("El numero de apariciones de la letra %c es %d\n", L, count);
    free(cadena);
    return count;
}

int algoMPI(int argc, char** argv){
    
    int i, n, iterator, count=0; //count es el contador global (proceso 0)
    char *cadena;
    char L;
    int* arrayCount;
    int numprocs, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    printf("<ID: %d/%d> -- 1\n", rank, numprocs);
    int individualCount = 0;

    if (rank == 0){
        n = atoi(argv[1]); //numero de procesos
        L = *argv[2];
        arrayCount = malloc(sizeof(int) * numprocs);
        
        // n y L se envian con send al resto de procesos.
        for (i=1; i<numprocs; i++){ 
            MPI_Send(&n, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&L, 1, MPI_CHAR, i, 0, MPI_COMM_WORLD);
        }
    }
    else{ //el resto de procesos reciben n y L
        MPI_Recv(&n, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&L, 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    
    printf("<ID: %d/%d> -- 2\n", rank, numprocs);

    cadena = (char *) malloc(n*sizeof(char));
    inicializaCadena(cadena, n);
    
    for (iterator = 0; iterator < n; iterator += numprocs){
        //nos aseguramos de no salirnos del array
        if(iterator+rank < n && cadena[iterator + rank] == L){
          individualCount += 1;
        }
    }
    
    printf("<ID: %d/%d> -- 3\n", rank, numprocs);
    
    //receive x numprocs -1 -> 0
    if (rank == 0){
        arrayCount[0] = individualCount;
        for (i = 1; i < numprocs; i++){
            MPI_Recv(&arrayCount[i], 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        count = sumArray(arrayCount, numprocs);
    }
    //send-> proc hermanos
    else{
        MPI_Send(&individualCount, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
    
    printf("<ID: %d/%d> -- 4\n", rank, numprocs);
    
    if (rank == 0)
        printf("El numero de apariciones de la letra %c es %d\n", L, count);
    free(cadena);
    
    MPI_Finalize();
    return count;
}

void comparacion(){
    int i, j, n, numprocs;
    char L;   
    int countSec, countMPI;

    int n_values[] = {100, 1000, 10000};
    int numprocs_values[] = {2, 4, 8};
    char L_values[] = {'A', 'C', 'G', 'T'};

    for(i = 0; i < 3; i++){
        n = n_values[i];
        for(j=0; j<3; j++){
            numprocs = numprocs_values[j];
            for(L = 'A'; L <= 'T'; L++){
                // Genera la cadena de entrada
                char *cadena = (char *) malloc(n*sizeof(char));
                inicializaCadena(cadena, n);

                // Ejecuta los algoritmos
                countSec = algoSec(cadena, n, L);
                countMPI = algoMPI(cadena, n, L, numprocs);

                // Compara los resultados
                if (countSec != countMPI) {
                    printf("ERROR: Los resultados son diferentes para n=%d, L=%c, numprocs=%d\n", n, L, numprocs);
                }

                free(cadena);
            }
        }
    }
}



int main(int argc, char *argv[]){
    //Aqui queda hacer una interfaz para elegir entre algoMPI y algoSec, y hacer pruebas con varias combinaciones de longitud de cadena y num de proc
    
    comparacion();

    return 0;
}

