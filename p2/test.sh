#! /bin/bash

#Ejecutamos este script en terminal escribiendo: bash test.sh

mpicc codigo_paralelo.c -o codigo_paralelo
gcc codigo_secuencial.c -o codigo_secuencial

# Definimos los parametros que van a variar
length=(10 13 21 100)
num_procs=(2 4 6)
letter=('A' 'C' 'G' 'T')

for l in "${length[@]}"
do
    for a in "${letter[@]}"
    do 
        echo -e "Probamos codigo con n = $l, contando la letra $a"

        #Ejecutamos el alg_secuencial
        ./codigo_secuencial $l $a
        for p in "${num_procs[@]}"
        do
            #Ejecutamos el alg_paralelo 
            mpirun -np $p codigo_paralelo $l $a
        done
        echo ""
    done
done