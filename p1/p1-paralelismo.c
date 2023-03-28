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

int main(int argc, char *argv[])
{
    if(argc != 3){
        printf("Numero incorrecto de parametros\nLa sintaxis debe ser: program n L\n  program es el nombre del ejecutable\n  n es el tamaño de la cadena a generar\n  L es la letra de la que se quiere contar apariciones (A, C, G o T)\n");
        exit(1); 
    }
    
    int i, n, iterator, count=0; //count es el contador global (proceso 0)
    char *cadena;
    char L;
    int* arrayCount;
    int numprocs, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    
    int individualCount = 0;

    if (my_id == 0){
        n = atoi(argv[1]); //numero de procesos
        L = *argv[2];
        arrayCount = malloc(sizeof(int) * numprocs);
    }
    
    // n y L se envian con send al resto de procesos. //////////////// PENDIENTE ///////////////////////  

    cadena = (char *) malloc(n*sizeof(char));
    inicializaCadena(cadena, n);
    
    
    for (iterator = 0; iterator < n; iterator += numprocs){
        if(cadena[iterator + rank] == L){
          individualCount += 1; //aqui hay que usar otro contador
        }
    }
    
    //receive x 3 -> 0
    if (my_id == 0){
        for (i = 0; i < numprocs; i++){
            MPI_Recv(&arrayCount[i], 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        count = sumArray(arrayCount, numprocs);
    }
    //send x 3 -> 1,2,3
    else{
        ////////////// SIN TERMINAR ///////////////
        MPI_Send(&individualCount, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
    
  
    printf("El numero de apariciones de la letra %c es %d\n", L, count);
    free(cadena);
    exit(0);
}

