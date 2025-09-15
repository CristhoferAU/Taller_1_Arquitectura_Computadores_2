#!/bin/bash
set -e

mkdir -p results

# Lista de subproyectos OpenMP
subproyectos=("OpenMP_Histograma_Atomic" "OpenMP_Histograma_Critical" "OpenMP_Histograma_Privado")

# Compilar y ejecutar
for subdir in "${subproyectos[@]}"; do
    echo "=== Procesando $subdir ==="

    cd "OpenMP/$subdir"

    # Compilar
    src_file=$(ls *.cpp)
    g++ -o bench "$src_file" -fopenmp

    # Ejecutar benchmark
    pwd
    mkdir -p ../../results/"$subdir"
    echo "hilos,tiempo" > ../../results/"$subdir"/tiempos_perf.csv

    for h in 1 2; do
        /usr/lib/linux-tools/6.8.0-79-generic/perf stat -e cycles,instructions,cache-misses -o temp.txt ./bench $h
        tiempo=$(grep "seconds time elapsed" temp.txt | awk '{print $1}')
        echo "$h,$tiempo" >> ../../results/"$subdir"/tiempos_perf.csv
    done

    cd ../..
done

# Graficar todos los resultados
python3 graficar.py
