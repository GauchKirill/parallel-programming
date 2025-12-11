import matplotlib.pyplot as plt
import numpy as np

# Пример данных (замените на свои измерения)
num_executors = [1, 2, 3, 4, 5, 6, 7, 8]
times_1z_omp = [0.319217, 0.190272, 0.155184, 0.152061, 0.155828, 0.155466, 0.149250, 0.152830]
times_2z_omp = [0.172096, 0.140691, 0.090951, 0.076840, 0.085022, 0.077103, 0.070406, 0.063828]
times_3z_omp = [0.183003, 0.275210, 0.198870, 0.157596, 0.195305, 0.169585, 0.167007, 0.158160]

def compute_speedup(times):
    return [times[0] / t for t in times]

def compute_efficiency(speedup, num_executors):
    return [s / p for s, p in zip(speedup, num_executors)]

speedup_1z_omp = compute_speedup(times_1z_omp)
speedup_2z_omp = compute_speedup(times_2z_omp)
speedup_3z_omp = compute_speedup(times_3z_omp)

eff_1z_omp = compute_efficiency(speedup_1z_omp, num_executors)
eff_2z_omp = compute_efficiency(speedup_2z_omp, num_executors)
eff_3z_omp = compute_efficiency(speedup_3z_omp, num_executors)

# График ускорения
plt.figure(figsize=(10, 6))
plt.plot(num_executors, speedup_1z_omp, marker='o', label='1з OpenMP')
plt.plot(num_executors, speedup_2z_omp, marker='s', label='2з OpenMP')
plt.plot(num_executors, speedup_3z_omp, marker='^', label='3з OpenMP')
plt.plot(num_executors, num_executors, 'k--', label='Идеальное ускорение')
plt.xlabel('Количество исполнителей')
plt.ylabel('Ускорение')
plt.title('Ускорение параллельных программ')
plt.legend()
plt.grid(True)
plt.savefig('speedup.png')
plt.show()

# График эффективности
plt.figure(figsize=(10, 6))
plt.plot(num_executors, eff_1z_omp, marker='o', label='1з OpenMP')
plt.plot(num_executors, eff_2z_omp, marker='s', label='2з OpenMP')
plt.plot(num_executors, eff_3z_omp, marker='^', label='3з OpenMP')
plt.xlabel('Количество исполнителей')
plt.ylabel('Эффективность')
plt.title('Эффективность параллельных программ')
plt.legend()
plt.grid(True)
plt.savefig('efficiency.png')
plt.show()