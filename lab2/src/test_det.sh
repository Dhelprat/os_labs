#!/bin/bash

# Компилируем программу
g++ -o matrix_determinant main.cpp -pthread

# Проверяем, что компиляция прошла успешно
if [ $? -ne 0 ]; then
    echo "Ошибка компиляции."
    exit 1
fi

# Размеры матриц для тестирования
MATRIX_SIZES=(4 8 10 11 12)
# Количество потоков для тестирования
THREAD_COUNTS=(1 2 3 4 5 6 7 8 9 10 11 12)

# Заголовок таблицы результатов
echo "Размер_матрицы Макс_потоков Время_мс      Ускорение Эффективность"

# Функция для измерения времени, работающая на разных платформах
get_time_ms() {
    if [[ "$OSTYPE" == "darwin"* ]]; then
        # MacOS - используем perl для точного измерения времени
        perl -MTime::HiRes=time -e 'printf "%.6f\n", time'
    else
        # Linux
        date +%s.%N
    fi
}

# Для каждого размера матрицы
for size in "${MATRIX_SIZES[@]}"; do
    # Сохраняем время выполнения для одного потока, чтобы рассчитать ускорение
    single_thread_time=0

    # Для каждого количества потоков
    for threads in "${THREAD_COUNTS[@]}"; do
        # Если количество потоков превышает размер матрицы, пропускаем
        # (т.к. программа ограничивает фактическое число потоков размером матрицы)
        if [ $threads -gt $size ]; then
            continue
        fi

        # Запускаем программу и замеряем время
        start_time=$(get_time_ms)
        ./matrix_determinant $size $threads > /dev/null 2>&1
        end_time=$(get_time_ms)

        # Вычисляем время выполнения в миллисекундах
        elapsed_time=$(echo "($end_time - $start_time) * 1000" | bc)
        elapsed_ms=$(printf "%.2f" $elapsed_time)

        # Для одного потока сохраняем время как базовое
        if [ $threads -eq 1 ]; then
            single_thread_time=$elapsed_time
        fi

        # Вычисляем ускорение и эффективность
        speedup=$(echo "$single_thread_time / $elapsed_time" | bc -l)
        speedup=$(printf "%.2f" $speedup)

        efficiency=$(echo "$speedup / $threads" | bc -l)
        efficiency=$(printf "%.2f" $efficiency)

        # Выводим результаты
        printf "%-15s %-13s %-14s %-10.2f %-13.2f\n" "$size" "$threads" "$elapsed_ms" "$speedup" "$efficiency"
    done
    echo "-------------------------------------------------------------"
done