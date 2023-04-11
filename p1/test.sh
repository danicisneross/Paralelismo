#! /bin/bash

#Ejecutamos este script en terminal escribiendo: bash test.sh

mpicc codigo_paralelo.c -o codigo_paralelo
gcc codigo_secuencial.c -o p1-secuencial

# Definimos los parametros que van a variar
length=(10 100 1000 10000)
num_procs=(2 4 8 10)
letter=('A' 'C' 'G' 'T')

for l in "${length[@]}"
do 
    for p in "${num_procs[@]}"
    do 
        for a in "${letter[@]}"
        do 
            echo "Probamos con n = $l y $p procesos, contando la letra $a"

            #Ejecutamos el alg_secuencial
            ./codigo_secuencial $l $a

            #Ejecutamos el alg_paralelo 
            mpirun -np $p codigo_paralelo $l $a 

            echo ""
        done
    done    
done