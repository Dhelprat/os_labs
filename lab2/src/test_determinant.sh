#!/bin/bash

# Компилируем программу
g++ -o matrix_determinant main.cpp -pthread

# Размеры матриц для тестирования
MATRIX_SIZES=(8 16 32 64)
# Количество потоков для тестирования
THREAD_COUNTS=(1 2 3 4 5 6)

echo "Размер_матрицы Макс_потоков Время_мс Ускорение_Eff Эффективность_Eff"

for SIZE in "${MATRIX_SIZES[@]}"; do
    T1=0  # Время выполнения для 1 потока

    for THREADS in "${THREAD_COUNTS[@]}"; do
        # Генерация случайной матрицы
        MATRIX_FILE="matrix_${SIZE}.txt"
        echo $SIZE > $MATRIX_FILE
        for ((i = 0; i < SIZE; i++)); do
            for ((j = 0; j < SIZE; j++)); do
                echo -n "$((RANDOM % 10)) " >> $MATRIX_FILE
            done
            echo "" >> $MATRIX_FILE
        done

        # --- Измерение времени (универсально для MacOS/Linux) ---
        START_SEC=$(date +%s)
        START_NANO=$(date +%N)

        if [[ "$START_NANO" == *N ]]; then
            START_NANO=0
        else
            START_NANO=$(($START_NANO / 1000000))
        fi

        START_TIME=$((START_SEC * 1000 + START_NANO))

        ./matrix_determinant $THREADS < $MATRIX_FILE > /dev/null

        END_SEC=$(date +%s)
        END_NANO=$(date +%N)

        if [[ "$END_NANO" == *N ]]; then
            END_NANO=0
        else
            END_NANO=$(($END_NANO / 1000000))
        fi

        END_TIME=$((END_SEC * 1000 + END_NANO))

        EXEC_TIME=$((END_TIME - START_TIME))  # Разница в миллисекундах

        # Сохранение времени для 1 потока
        if [ "$THREADS" -eq 1 ]; then
            T1=$EXEC_TIME
        fi

        # Вычисление ускорения и эффективности с нормальным форматом
        if [ "$THREADS" -eq 1 ]; then
            S_N=1.00
            E_N=1.00
        else
            S_N=$(echo "scale=2; $T1 / $EXEC_TIME" | bc)
            E_N=$(echo "scale=2; $S_N / $THREADS" | bc)
        fi

        # Вывод с форматированием (включает ведущие нули)
        printf "%-15s %-15s %-15s %-15.2f %-15.2f\n" "$SIZE" "$THREADS" "$EXEC_TIME" "$S_N" "$E_N"
    done
done
