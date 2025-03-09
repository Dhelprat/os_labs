# Используем официальный образ Ubuntu 22.04
FROM ubuntu:22.04

# Установка необходимых пакетов
RUN apt-get update && \
    apt-get install -y \
    build-essential \
    g++ \
    cmake \
    strace \
    gdb \
    git \
    && rm -rf /var/lib/apt/lists/*

# Устанавливаем рабочую директорию
WORKDIR /workspace

# Копируем исходный код в контейнер
COPY . .

# Команда по умолчанию (можно изменить)
CMD ["bash"]