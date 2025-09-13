#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <omp.h>

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

// Histograma paralelo: privados + reducción final
std::vector<size_t> histograma_privados(const std::vector<int>& datos, int min_val, int max_val) {
    size_t rango = max_val - min_val + 1;
    std::vector<size_t> hist_global(rango, 0);

    int num_threads;
    #pragma omp parallel
    {
        num_threads = omp_get_num_threads();
        std::vector<size_t> hist_local(rango, 0);

        #pragma omp for nowait
        for (size_t i = 0; i < datos.size(); i++) {
            hist_local[datos[i] - min_val]++;
        }

        // Reducción final
        #pragma omp critical
        {
            for (size_t j = 0; j < rango; j++) {
                hist_global[j] += hist_local[j];
            }
        }
    }

    return hist_global;
}

int main() {
    size_t N = 100000000;
    int min_val = 0;
    int max_val = 255;
    unsigned int seed = 42;

    auto start = std::chrono::high_resolution_clock::now();
    std::vector<int> datos = generar_datos(N, min_val, max_val, seed);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> tiempo_gen = end - start;
    std::cout << "Generados " << N << " numeros en " << tiempo_gen.count() << " segundos\n";

    // Histograma paralelo (variante A)
    start = std::chrono::high_resolution_clock::now();
    std::vector<size_t> hist = histograma_privados(datos, min_val, max_val);
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> tiempo_hist = end - start;

    // Validación
    size_t suma = 0;
    for (auto v : hist) suma += v;

    std::cout << "Histograma PRIVADOS+REDUCCION en " << tiempo_hist.count() << " segundos\n";
    std::cout << "Suma total en histograma = " << suma << " (esperado: " << N << ")\n";

    return 0;
}

