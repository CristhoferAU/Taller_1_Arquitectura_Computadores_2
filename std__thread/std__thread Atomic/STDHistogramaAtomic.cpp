#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <thread>
#include <atomic>

// Genera datos aleatorios (secuencial)
std::vector<int> generar_datos(size_t N, int min_val, int max_val, unsigned int seed) {
    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> dist(min_val, max_val);

    std::vector<int> datos(N);
    for (size_t i = 0; i < N; i++) {
        datos[i] = dist(gen);
    }
    return datos;
}

// Función que procesa un bloque de datos y actualiza el histograma global con atomics
void calcular_histograma_atomic(const std::vector<int>& datos, int min_val,
                                size_t inicio, size_t fin,
                                std::vector<std::atomic<size_t>>& hist_global) {
    for (size_t i = inicio; i < fin; i++) {
        int valor = datos[i] - min_val;
        hist_global[valor].fetch_add(1, std::memory_order_relaxed);
    }
}

// Histograma paralelo con atomic
std::vector<size_t> histograma_global_atomic(const std::vector<int>& datos, int min_val, int max_val, int num_threads) {
    size_t rango = max_val - min_val + 1;
    std::vector<std::atomic<size_t>> hist_global(rango);
    for (size_t i = 0; i < rango; i++) hist_global[i] = 0;

    std::vector<std::thread> hilos;
    size_t chunk = datos.size() / num_threads;

    for (int t = 0; t < num_threads; t++) {
        size_t inicio = t * chunk;
        size_t fin = (t == num_threads - 1) ? datos.size() : (t + 1) * chunk;

        hilos.emplace_back(calcular_histograma_atomic,
                           std::cref(datos), min_val,
                           inicio, fin, std::ref(hist_global));
    }

    for (auto& h : hilos) h.join();

    // Convertir de atomic<size_t> a size_t normal
    std::vector<size_t> hist_final(rango);
    for (size_t i = 0; i < rango; i++) {
        hist_final[i] = hist_global[i].load();
    }

    return hist_final;
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
    std::cout << "Generados " << N << " numeros en " << tiempo_gen.count() << " segundos (std::mt19937 secuencial)\n";

    int num_threads = std::thread::hardware_concurrency();
    std::cout << "Usando " << num_threads << " threads\n";

    start = std::chrono::high_resolution_clock::now();
    std::vector<size_t> hist = histograma_global_atomic(datos, min_val, max_val, num_threads);
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> tiempo_hist = end - start;

    size_t suma = 0;
    for (auto v : hist) suma += v;

    std::cout << "Histograma GLOBAL+ATOMIC (std::thread) en " << tiempo_hist.count() << " segundos\n";
    std::cout << "Suma total en histograma = " << suma << " (esperado: " << N << ")\n";

    return 0;
}

