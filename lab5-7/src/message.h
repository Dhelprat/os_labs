#pragma once

#include <cstring>
#include <chrono>

// Структура сообещения, которым будут обмениваться ноды
class Message {
public:
    Message() = default;

    // Сообщения без key
    Message(const char* cmd, int id, int add_data)
        : receiver_id(id)
        , additional_data(add_data)
        , sent_time(std::chrono::system_clock::now())
    {
        std::strncpy(command, cmd, sizeof(command) - 1);
        command[sizeof(command) - 1] = '\0';
    }

    // Сообщения с key
    Message(const char* cmd, int id, int add_data, const char* s)
        : receiver_id(id)
        , additional_data(add_data)
        , sent_time(std::chrono::system_clock::now())
    {
        std::strncpy(command, cmd, sizeof(command) - 1);
        command[sizeof(command) - 1] = '\0';

        std::strncpy(key, s, sizeof(key) - 1);
        key[sizeof(key) - 1] = '\0';
    }

    // Все поля равны => сообщения равны
    bool operator==(const Message& other) const {
        return strcmp(command,other.command) == 0 &&
               receiver_id == other.receiver_id &&
               additional_data == other.additional_data &&
               sent_time == other.sent_time;
    }

    char command[10]{};
    int receiver_id{-1};
    int additional_data{-1};
    std::chrono::system_clock::time_point sent_time;
    char key[30]{};
};

