#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <thread>

// Función que genera datos secuenciales (sin OpenMP)
std::vector<int> generar_datos(size_t N, int min_val, int max_val, unsigned int seed) {
    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> dist(min_val, max_val);

    std::vector<int> datos(N);
    for (size_t i = 0; i < N; i++) {
        datos[i] = dist(gen);
    }
    return datos;
}

// Función que calcula un histograma parcial
void calcular_histograma_parcial(const std::vector<int>& datos, int min_val, int max_val,
                                 size_t inicio, size_t fin, std::vector<size_t>& hist_local, unsigned int seed) {
    std::mt19937 gen(seed);  // Si quisieras random aquí, aunque no aplica en histograma
    for (size_t i = inicio; i < fin; i++) {
        hist_local[datos[i] - min_val]++;
    }
}

// Histograma paralelo con histogramas privados + reducción final
std::vector<size_t> histograma_privados(const std::vector<int>& datos, int min_val, int max_val, int num_threads) {
    size_t rango = max_val - min_val + 1;
    std::vector<size_t> hist_global(rango, 0);

    // Crear vectores locales (uno por hilo)
    std::vector<std::vector<size_t>> histogramas_locales(num_threads, std::vector<size_t>(rango, 0));
    std::vector<std::thread> hilos;

    size_t chunk = datos.size() / num_threads;

    // Lanzar hilos
    for (int t = 0; t < num_threads; t++) {
        size_t inicio = t * chunk;
        size_t fin = (t == num_threads - 1) ? datos.size() : (t + 1) * chunk;

        hilos.emplace_back(calcular_histograma_parcial,
                           std::cref(datos), min_val, max_val,
                           inicio, fin, std::ref(histogramas_locales[t]), 42 + t);
    }

    // Esperar a que terminen
    for (auto& h : hilos) h.join();

    // Reducción final
    for (int t = 0; t < num_threads; t++) {
        for (size_t j = 0; j < rango; j++) {
            hist_global[j] += histogramas_locales[t][j];
        }
    }

    return hist_global;
}

int main() {
    size_t N = 100000000;
    int min_val = 0;
    int max_val = 255;
    unsigned int seed = 42;

    std::vector<int> datos = generar_datos(N, min_val, max_val, seed);
    std::vector<size_t> hist = histograma_privados(datos, min_val, max_val, num_threads);

    size_t suma = 0;
    for (auto v : hist) suma += v;

    return 0;
}

