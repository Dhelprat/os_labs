FROM ubuntu:22.04

# Обновление пакетов и установка зависимостей
RUN apt-get update && \
    apt-get install -y g++ make strace && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .