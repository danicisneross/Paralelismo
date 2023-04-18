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

void algoMPI(int argc, char** argv){
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
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs); //obtenemos el numero de procesos
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); //obtenemos el identificador de cada proceso

    //printf("<ID: %d/%d> -- 1\n", rank, numprocs);
    int individualCount = 0;

    if (rank == 0){ //proceso principal
        n = atoi(argv[1]); //numero de procesos
        L = *argv[2]; //letra a contar
        arrayCount = malloc(sizeof(int) * numprocs);
        
        // n y L se envian con send al resto de procesos.
        for (i=1; i<numprocs; i++){ 
            /*int MPI_Send ( void * buff , int count , MPI_Datatype datatype ,
int dest , int tag , MPI_Comm comm ) ;
	    -COUNT --> num elementos a enviar
	    -i --> id de procesor que reciben*/
        
            MPI_Send(&n, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&L, 1, MPI_CHAR, i, 0, MPI_COMM_WORLD);
        }
    }
    else{ //el resto de procesos reciben n y L
    	/*int MPI_Recv ( void * buff , int count , MPI_Datatype datatype ,int source , int tag , MPI_Comm comm , MPI_Status * status ); 
    	-COUNT --> num de elementos que se esperan recibir
    	-MPI_INT --> tipo de dato que se espera recibir (en este caso un entero)
    	-source --> id del proceso que envia el mensaje
    	*/
    	
        MPI_Recv(&n, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&L, 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    
    //printf("<ID: %d/%d> -- 2\n", rank, numprocs);

    cadena = (char *) malloc(n*sizeof(char));
    inicializaCadena(cadena, n);
    
    for (iterator = 0; iterator < n; iterator += numprocs){ //iterator aunmenta en incrementos iguales al numero de procesos???  
        //nos aseguramos de no salirnos del array
        if(iterator+rank < n && cadena[iterator + rank] == L){
          individualCount += 1;
        }
    }
    
    //printf("<ID: %d/%d> -- 3\n", rank, numprocs);
    
    //receive x numprocs -1 -> 0
    if (rank == 0){
        arrayCount[0] = individualCount;
        for (i = 1; i < numprocs; i++){
       	    //Recibimos los valores de los procesos secundarios
            MPI_Recv(&arrayCount[i], 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        count = sumArray(arrayCount, numprocs); //sumamos todos los resultados de todos los procesos
    }
    //send-> proc hermanos
    else{
        MPI_Send(&individualCount, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
    
    //printf("<ID: %d/%d> -- 4\n", rank, numprocs);
    
    if (rank == 0) //llegamos a que recibio todos los resultados de los demas procesos y por ende podemos imprimir el conteo total
        printf("MPI (%d): El numero de apariciones de la letra %c es %d\n", numprocs, L, count);
    free(cadena);
    
    MPI_Finalize(); //finalizamos el ambiante MPI --> liberamos los recursos asociados con MPI, como los comunicadores, los grupos de procesos y los buffers de comunicación.
}

int main(int argc, char *argv[]){
    //Llamamos al algoritmo paralelo
    algoMPI(argc, argv);
}
