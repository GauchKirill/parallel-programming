import numpy as np
import matplotlib.pyplot as plt
import glob
import os
import re

def read_solution_file(filename):
    """Чтение файла с решением"""
    try:
        with open(filename, 'r') as f:
            lines = f.readlines()
        
        # Извлекаем значение параметра a
        a_value = None
        for line in lines[:3]:
            if line.startswith('# a ='):
                try:
                    a_value = float(line.split('=')[1].strip())
                    break
                except:
                    pass
        
        # Если не нашли в заголовке, берем из имени файла
        if a_value is None:
            match = re.search(r'solution_a_([0-9\.eE\+\-]+)\.txt', filename)
            if match:
                try:
                    a_value = float(match.group(1))
                except:
                    pass
        
        # Читаем данные
        x_data = []
        y_data = []
        for line in lines:
            line = line.strip()
            if line and not line.startswith('#'):
                parts = line.split()
                if len(parts) >= 2:
                    try:
                        x = float(parts[0])
                        y = float(parts[1])
                        if not (np.isnan(x) or np.isnan(y) or np.isinf(x) or np.isinf(y)):
                            x_data.append(x)
                            y_data.append(y)
                    except:
                        continue
        
        return a_value, np.array(x_data), np.array(y_data)
    
    except Exception as e:
        print(f"Error reading {filename}: {e}")
        return None, None, None

def plot_solutions_grid():
    """Построение сетки графиков для всех решений"""
    solution_files = glob.glob("solution_a_*.txt")
    
    if not solution_files:
        print("No solution files found!")
        return 0
    
    print(f"Found {len(solution_files)} solution files")
    
    # Сортируем файлы по значению a
    solution_files.sort(key=lambda x: float(re.search(r'solution_a_([0-9\.eE\+\-]+)\.txt', x).group(1)))
    
    # Определяем количество графиков
    n_files = len(solution_files)
    n_cols = min(4, n_files)
    n_rows = (n_files + n_cols - 1) // n_cols
    
    fig, axes = plt.subplots(n_rows, n_cols, figsize=(4*n_cols, 3*n_rows))
    
    if n_rows == 1 and n_cols == 1:
        axes = np.array([axes])
    elif n_rows == 1:
        axes = axes.reshape(1, -1)
    elif n_cols == 1:
        axes = axes.reshape(-1, 1)
    
    successful_plots = 0
    
    for idx, filename in enumerate(solution_files):
        row = idx // n_cols
        col = idx % n_cols
        
        a_value, x, y = read_solution_file(filename)
        
        if a_value is not None and x is not None and y is not None and len(x) > 10:
            ax = axes[row, col]
            
            # Очищаем выбросы
            mask = ~(np.isnan(y) | np.isinf(y))
            x_clean = x[mask]
            y_clean = y[mask]
            
            if len(x_clean) > 10:
                ax.plot(x_clean, y_clean, 'b-', linewidth=1.5)
                ax.set_xlabel('x')
                ax.set_ylabel('y(x)')
                ax.set_title(f'a = {a_value:.2e}', fontsize=10)
                ax.grid(True, alpha=0.3)
                
                # Добавляем линию на y = sqrt(2)
                sqrt2 = np.sqrt(2)
                ax.axhline(y=sqrt2, color='r', linestyle='--', alpha=0.5, linewidth=0.8)
                
                successful_plots += 1
            else:
                ax.text(0.5, 0.5, 'Invalid data', ha='center', va='center', transform=ax.transAxes)
                ax.set_title(f'a = {a_value:.2e}', fontsize=10)
        else:
            ax = axes[row, col]
            ax.text(0.5, 0.5, 'No data', ha='center', va='center', transform=ax.transAxes)
            ax.set_title('Failed')
    
    # Скрываем пустые оси
    for idx in range(len(solution_files), n_rows * n_cols):
        row = idx // n_cols
        col = idx % n_cols
        axes[row, col].axis('off')
    
    plt.suptitle('Solutions of y\'\' = a(y - y³) with y(-10) = y(10) = √2\n(8 threads per value of a)', fontsize=14)
    plt.tight_layout()
    plt.savefig('solutions_grid.png', dpi=150, bbox_inches='tight')
    plt.show()
    
    return successful_plots

def plot_comparison():
    """Сравнительный график всех успешных решений"""
    solution_files = glob.glob("solution_a_*.txt")
    
    if not solution_files:
        return
    
    solution_files.sort(key=lambda x: float(re.search(r'solution_a_([0-9\.eE\+\-]+)\.txt', x).group(1)))
    
    plt.figure(figsize=(12, 8))
    
    successful = 0
    
    for filename in solution_files:
        a_value, x, y = read_solution_file(filename)
        
        if a_value is not None and x is not None and y is not None and len(x) > 10:
            # Очищаем данные
            mask = ~(np.isnan(y) | np.isinf(y))
            x_clean = x[mask]
            y_clean = y[mask]
            
            if len(x_clean) > 10:
                plt.plot(x_clean, y_clean, linewidth=1.5, alpha=0.7, label=f'a = {a_value:.2e}')
                successful += 1
    
    if successful > 0:
        sqrt2 = np.sqrt(2)
        plt.axhline(y=sqrt2, color='k', linestyle='--', alpha=0.7, linewidth=1.5, label=f'y = √2 ≈ {sqrt2:.3f}')
        
        plt.xlabel('x', fontsize=12)
        plt.ylabel('y(x)', fontsize=12)
        plt.title('Comparison of Solutions for Different a Values\n(Parallel reduction + Thomas algorithm)', fontsize=14)
        plt.grid(True, alpha=0.3)
        
        if successful <= 10:
            plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left', fontsize=9)
        else:
            plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left', fontsize=8, ncol=2)
        
        plt.tight_layout()
        plt.savefig('comparison.png', dpi=150, bbox_inches='tight')
        plt.show()
    else:
        print("No valid data for comparison plot")

def create_statistics_report():
    """Создание отчета со статистикой"""
    solution_files = glob.glob("solution_a_*.txt")
    
    if not solution_files:
        print("No solution files found!")
        return
    
    solution_files.sort(key=lambda x: float(re.search(r'solution_a_([0-9\.eE\+\-]+)\.txt', x).group(1)))
    
    report_lines = []
    report_lines.append("=" * 70)
    report_lines.append("STATISTICS REPORT: y'' = a(y - y³), y(-10) = y(10) = √2")
    report_lines.append("=" * 70)
    report_lines.append("")
    report_lines.append(f"Number of solution files: {len(solution_files)}")
    report_lines.append("")
    
    valid_count = 0
    for filename in solution_files:
        a_value, x, y = read_solution_file(filename)
        
        if a_value is None or x is None or y is None:
            continue
        
        mask = ~(np.isnan(y) | np.isinf(y))
        x_clean = x[mask]
        y_clean = y[mask]
        
        if len(x_clean) < 10:
            continue
        
        valid_count += 1
        
        # Основные статистики
        center_idx = np.argmin(np.abs(x_clean))
        y_center = y_clean[center_idx]
        y_min, y_max = np.min(y_clean), np.max(y_clean)
        y_mean = np.mean(y_clean)
        y_std = np.std(y_clean)
        
        report_lines.append(f"a = {a_value:12.2e}:")
        report_lines.append(f"  Points: {len(x_clean)}")
        report_lines.append(f"  y(0) = {y_center:10.6f}")
        report_lines.append(f"  Range: [{y_min:8.6f}, {y_max:8.6f}]")
        report_lines.append(f"  Mean ± Std: {y_mean:8.6f} ± {y_std:8.6f}")
        report_lines.append("")
    
    report_lines.append(f"Valid solutions: {valid_count}/{len(solution_files)}")
    report_lines.append("=" * 70)
    
    # Сохраняем отчет
    with open("statistics_report.txt", "w") as f:
        f.write("\n".join(report_lines))
    
    print(f"\nStatistics report saved to statistics_report.txt")
    print(f"Valid solutions: {valid_count}/{len(solution_files)}")

def main():
    """Основная функция"""
    print("Visualizing solutions of y'' = a(y - y³)")
    print("Using reduction algorithm + parallel Thomas method (8 threads per a)")
    print("=" * 60)
    
    # Построение графиков
    print("\n1. Plotting solutions grid...")
    plot_solutions_grid()
    
    print("\n2. Plotting comparison graph...")
    plot_comparison()
    
    # Создание отчета
    print("\n3. Generating statistics report...")
    create_statistics_report()
    
    print("\nAll visualizations completed!")

if __name__ == "__main__":
    main()