#include <iostream>
#include <vector>
#include <random>
#include <omp.h>
#include <fstream>

// Genera datos aleatorios en paralelo
std::vector<int> generar_datos(size_t N, int min_val, int max_val, unsigned int seed) {
    std::vector<int> datos(N);

    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        std::mt19937 gen(seed + thread_id);
        std::uniform_int_distribution<int> dist(min_val, max_val);

        #pragma omp for
        for (size_t i = 0; i < N; i++) {
            datos[i] = dist(gen);
        }
    }
    return datos;
}

// Histograma paralelo con variables atómicas
std::vector<size_t> histograma_global_atomic(const std::vector<int>& datos, int min_val, int max_val) {
    size_t rango = max_val - min_val + 1;
    std::vector<size_t> hist_global(rango, 0);

    #pragma omp parallel for
    for (size_t i = 0; i < datos.size(); i++) {
        int valor = datos[i] - min_val;
        #pragma omp atomic
        hist_global[valor]++;
    }

    return hist_global;
}

int main(int argc, char* argv[]) {
    size_t N = 100000000;
    int min_val = 0;
    int max_val = 255;
    unsigned int seed = 42;

    int num_threads = (argc > 1) ? std::stoi(argv[1]) : omp_get_max_threads();
    omp_set_num_threads(num_threads);

    std::vector<int> datos = generar_datos(N, min_val, max_val, seed);
    std::vector<size_t> hist = histograma_global_atomic(datos, min_val, max_val);

    size_t suma = 0;
    for (auto v : hist) suma += v;

    return 0;
}
