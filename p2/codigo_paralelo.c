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

int MPI_BinomialBcast (void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm){
    int numprocs, i, rank, power, err;

    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    /*El número de iteraciones que necesitamos*/
    int pasos = ceil(log10(numprocs)/log10(2));

    for (i=1; i <= pasos; i++){
        /* Receptor = rank -> emisor = rank - power
           Emisor = rank -> receptor = rank + power */
        power = pow(2, i-1);
        if (rank < power){
            //Send
            if (rank + power < numprocs){ //Salvaguarda
                err = MPI_Send(buffer, count, datatype, rank + power, 0, comm);
                if (err != 0) //Control del error
                    return err;
            }
        }
        else if (rank < pow(2, i)){
            //Receive
            /*no es necesario un salvaguarda (recibimos de un rank menor al 
            nuestro*/
            err = MPI_Recv(buffer, count, datatype, rank - power, 0, comm, MPI_STATUS_IGNORE);
            if (err != 0) //Control del error
                return err;
        }
    }

    return 0;
}

int MPI_FlattreeColectiva(void *sendbuf, void *recvbuf, int count,
    MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm){
    int numprocs, i, rank, power, res = 0, err;
    //No modificamos sendbuf, trabajamos con una copia
    int* buf = (int*) sendbuf;
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int pasos = ceil(log10(numprocs)/log10(2));

    //Realizamos la op inversa a BinomialBcast
    for (i = pasos; i >= 1; i--){
        /* Receptor = rank -> emisor = rank + power
           Emisor = rank -> receptor = rank - power */
        power = pow(2, i-1);

        if (rank < power){
            //Recv
            if (rank + power < numprocs){ //Salvaguarda
                err = MPI_Recv(&res, count, datatype, rank + power, 0, comm, MPI_STATUS_IGNORE);
                if (err != 0){ //Control del error
                    printf("Error: %d\n", err);
                    return err;
                }
                *buf += res; //Sumamos a nuestro valor el valor recibido

                if (i == 1){ //i = 1 -> última iteración. Proc 0 escribe el res
                    *(int*) recvbuf = *buf;
                }
            }
        }
        else if (rank < pow(2, i)){
            //Send
            err = MPI_Send((void*) buf, count, datatype, rank - power, 0, comm);
            if (err != 0) //Control del error
                return err;
        }
    }

    return 0;
}

void algoMPI(int argc, char** argv){
    if(argc != 3){
        printf("Numero incorrecto de parametros\nLa sintaxis debe ser: program n L\n  program es el nombre del ejecutable\n  n es el tamaño de la cadena a generar\n  L es la letra de la que se quiere contar apariciones (A, C, G o T)\n");
        exit(1); 
    }
    
    int n, iterator, individualCount = 0, count=0; //count es el contador global (proceso 0)
    char *cadena;
    char L;
    int numprocs, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // El proceso 0 es el único que inicializa n y L con los argumentos
    if (rank == 0){
        n = atoi(argv[1]); //numero de procesos
        L = *argv[2];
    }

    //El proceso 0 envía el valor de n y L al resto de procesos
    MPI_BinomialBcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_BinomialBcast(&L, 1, MPI_CHAR, 0, MPI_COMM_WORLD);
    //MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    //MPI_Bcast(&L, 1, MPI_CHAR, 0, MPI_COMM_WORLD);

    //Cada proceso inicializa su propia cadena (con el mismo contenido)
    cadena = (char *) malloc(n*sizeof(char));
    inicializaCadena(cadena, n);
    
    /*Cada proceso cuenta el número de apariciones de una letra
    en un segmento de la cadena*/
    for (iterator = 0; iterator < n; iterator += numprocs){
        //nos aseguramos de no salirnos del array
        if(iterator+rank < n && cadena[iterator + rank] == L){
          individualCount += 1;
        }
    }
    
    /*El proceso 0 suma los resultados individuales de cada proceso y almacena
    el resultado en count*/
    MPI_FlattreeColectiva(&individualCount, &count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    //MPI_Reduce(&individualCount, &count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    
    //Imprime el resultado por pantalla
    if (rank == 0)
        printf("MPI (%d): El numero de apariciones de la letra %c es %d\n", numprocs, L, count);
    free(cadena);
    
    MPI_Finalize();
}

void funcion_prueba(int argc, char** argv){
    int numprocs, rank, n, res = 1;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0){
        n = 8;   
    }

    //Pasamos el valor de n al resto de procesos (n = 8)
    MPI_BinomialBcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    //Comprobamos que el valor es el deseado (todos deben imprimir el mismo)
    printf("Proceso %d, valor %d\n", rank, n);

    // Suma el valor de n de todos los procesos
    // En la variable res el proceso 0 debería tener n * numprocs
    MPI_FlattreeColectiva(&n, &res, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    //Comprobamos que el valor es correcto
    if (rank == 0)
        printf("Resultado: %d\n", res);

    MPI_Finalize();
}

int main(int argc, char *argv[]){
    //Llamamos al algoritmo paralelo
    algoMPI(argc, argv);
    //funcion_prueba(argc, argv);
}