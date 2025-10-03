#!/bin/bash

# Параметры для программы
sizes=(10 100 1024 10000 100000
  1000000 10000000 100000000
  1000000000)    # Размеры массива
processes=(1 2 3 4 5 6 7 8)         # Количество процессов MPI

# Исполняемые файлы и соответствующие файлы вывода
executables=(
  "sort_best"
  "sort_typical"
  "sort_worst"
)
output_files=(
  "exp_best.txt"
  "exp_typical.txt"
  "exp_worst.txt"
)

# Проверка, что количество исполняемых файлов совпадает с количеством файлов вывода
if [ ${#executables[@]} -ne ${#output_files[@]} ]; then
  echo "Ошибка: Количество исполняемых файлов не совпадает с количеством файлов вывода."
  exit 1
fi

# Главный цикл по исполняемым файлам
for i in "${!executables[@]}"; do
  executable="${executables[$i]}"
  output_file="${output_files[$i]}"

  # Проверка, существует ли исполняемый файл
  if [ ! -x "$executable" ]; then
    echo "Ошибка: исполняемый файл '$executable' не найден или не имеет прав на выполнение."
    continue # Переходим к следующему исполняемому файлу
  fi

  # Очистка файла результатов (если он существует)
  if [ -f "$output_file" ]; then
    rm "$output_file"
  fi

  # Цикл по размерам массива
  for size in "${sizes[@]}"; do
    # Цикл по количеству процессов
    for procs in "${processes[@]}"; do

      # Формируем команду mpirun, используя --use-hwthread-cpus
      mpirun -n "$procs" --use-hwthread-cpus "./$executable" "$size">> "$output_file"

      # Выводим информацию о текущем запуске
      echo "Запущено: mpirun -n $procs --use-hwthread-cpus ./$executable $size (размер: $size, процессов: $procs, вывод в: $output_file)"
    done
  done

  echo "Завершено для $executable. Результаты сохранены в файл: $output_file"
done

echo "Все задачи завершены."
