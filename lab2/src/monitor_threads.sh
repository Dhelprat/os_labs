#!/bin/bash

# Компилируем программу
g++ -pthread main.cpp -o matrix_determinant

# Запускаем программу в фоне
./matrix_determinant 11 6 &
PROG_PID=$!

echo "Program PID: $PROG_PID"
echo "Monitoring threads..."

# Мониторим каждую секунду
while kill -0 $PROG_PID 2>/dev/null; do
    # Используем ps для подсчета количества потоков
    thread_count=$(ps -M $PROG_PID | wc -l)
    # Вычитаем заголовок и сам процесс, чтобы получить только количество потоков
    thread_count=$((thread_count - 2))
    echo "$(date +%T%3N) - Number of threads: $thread_count"
    sleep 1
done

wait $PROG_PID