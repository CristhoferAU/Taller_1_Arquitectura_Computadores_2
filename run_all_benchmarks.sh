#!/bin/bash
set -e

mkdir -p results

# Detectar número máximo de hilos del sistema
MAX_THREADS=$(nproc)

# Función para generar lista de hilos a probar
generate_threads_list() {
    local max_threads=$1
    local threads=()
    local i=1
    while [ $i -le $max_threads ]; do
        threads+=($i)
        i=$((i*2))
    done
    if [[ ! " ${threads[@]} " =~ " ${max_threads} " ]]; then
        threads+=($max_threads)
    fi
    echo "${threads[@]}"
}

# Hilos a probar
hilos_to_test=($(generate_threads_list $MAX_THREADS))
echo "Hilos a probar: ${hilos_to_test[@]}"

# -------------------------
# Benchmark OpenMP
# -------------------------
subproyectos_openmp=("OpenMP_Histograma_Atomic" "OpenMP_Histograma_Critical" "OpenMP_Histograma_Privado")

for subdir in "${subproyectos_openmp[@]}"; do
    echo "=== Procesando OpenMP/$subdir ==="
    cd "OpenMP/$subdir"

    # Compilar
    src_file=$(ls *.cpp)
    g++ -o bench "$src_file" -fopenmp

    # Ejecutar benchmark
    mkdir -p ../../results/"$subdir"
    echo "hilos,tiempo" > ../../results/"$subdir"/tiempos_perf.csv

    for h in "${hilos_to_test[@]}"; do
        echo "Ejecutando $h hilos..."
        /usr/lib/linux-tools/6.8.0-79-generic/perf stat -e cycles,instructions,cache-misses -o temp.txt ./bench $h
        tiempo=$(grep "seconds time elapsed" temp.txt | awk '{print $1}')
        echo "$h,$tiempo" >> ../../results/"$subdir"/tiempos_perf.csv
    done

    cd ../..
done

# -------------------------
# Benchmark std::thread
# -------------------------
subproyectos_std=("std_thread Atomic" "std_thread Mutex" "std_thread Privado")

for subdir in "${subproyectos_std[@]}"; do
    echo "=== Procesando std_thread/$subdir ==="
    cd "std_thread/$subdir"

    # Compilar
    src_file=$(ls *.cpp)
    g++ -o bench "$src_file" -std=c++17 -pthread

    # Ejecutar benchmark
    mkdir -p ../../results/"$subdir"
    echo "hilos,tiempo" > ../../results/"$subdir"/tiempos_perf.csv

    for h in "${hilos_to_test[@]}"; do
        echo "Ejecutando $h hilos..."
        /usr/lib/linux-tools/6.8.0-79-generic/perf stat -e cycles,instructions,cache-misses -o temp.txt ./bench $h
        tiempo=$(grep "seconds time elapsed" temp.txt | awk '{print $1}')
        echo "$h,$tiempo" >> ../../results/"$subdir"/tiempos_perf.csv
    done

    cd ../..
done

# -------------------------
# Graficar todos los resultados
# -------------------------
python3 generate_graphs.py

echo "=== Todos los benchmarks completados ==="

python3 gui_benchmarks.py