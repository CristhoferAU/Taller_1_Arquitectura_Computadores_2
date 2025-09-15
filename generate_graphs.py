import matplotlib.pyplot as plt
import os
import csv

results_dir = "results"

for subdir in os.listdir(results_dir):
    subpath = os.path.join(results_dir, subdir)
    csv_file = os.path.join(subpath, "tiempos_perf.csv")
    
    if not os.path.exists(csv_file):
        continue

    # Leer CSV
    hilos, tiempos = [], []
    with open(csv_file) as f:
        reader = csv.reader(f)
        next(reader)
        for row in reader:
            hilos.append(int(row[0]))
            tiempos.append(float(row[1]))

    # Calcular speedup y eficiencia
    T1 = tiempos[0]
    speedup = [T1 / t for t in tiempos]
    eficiencia = [s / h for s, h in zip(speedup, hilos)]

    # Graficar función
    def plot_graph(x, y, xlabel, ylabel, title, filename, color):
        plt.figure()
        plt.plot(x, y, marker="o", linestyle="--", color=color)
        plt.xlabel(xlabel)
        plt.ylabel(ylabel)
        plt.title(title)
        plt.grid(True)
        plt.savefig(os.path.join(subpath, filename))
        plt.close()

    plot_graph(hilos, tiempos, "Número de hilos", "Tiempo (s)", "Tiempo vs Hilos", "tiempo.png", "blue")
    plot_graph(hilos, speedup, "Número de hilos", "Speedup", "Speedup vs Hilos", "speedup.png", "green")
    plot_graph(hilos, eficiencia, "Número de hilos", "Eficiencia", "Eficiencia vs Hilos", "eficiencia.png", "red")

    print(f"Gráficas generadas para {subdir}")
