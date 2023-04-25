#! /bin/bash

#Ejecutamos este script en terminal escribiendo: bash test.sh

mpicc similarity_paralelo.c -o similarity_paralelo
gcc similarity.c -o similarity

# Definimos los parametros que van a variar
num_procs=4

#Ejecutamos el alg_secuencial
    ./similarity

for p in "${num_procs[@]}"
do 
    #Ejecutamos el alg_paralelo 
    mpirun -np $p similarity_paralelo
done 
echo ""    
