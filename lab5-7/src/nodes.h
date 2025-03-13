#pragma once

#include "zmq.h"
#include "message.h"
#include <iostream>
#include <list>
#include <map>

// Базовая нода
class Node {
public:
    Node() = default;
    ~Node() {
        if (socket) zmq_close(socket);
        if (context) zmq_ctx_destroy(context);
    }

    Node(Node&& other) noexcept;

    bool operator==(const Node& other) const {
        return id == other.id && pid == other.pid;
    }

    int id{-1};
    pid_t pid{-1};
    void* context{nullptr};
    void* socket{nullptr};
    std::string address;
};

// Функции для создания детей
Node create_node(int id, bool is_child);
Node create_process(int id);

// Ребенок
class ComputingNode {
private:
    Node node_;
    std::map<std::string, int> dictionary_;
    std::list<Node> children_;

    void broadcast_to_children(const Message& message);

public:
    explicit ComputingNode(int id)
        : node_(create_node(id, true)) {}

    void run();
};


/*
Т.к хэдер используется в обоих файлах, и в них нужно отправлять и получать сообщения,
то, чтобы не плодить доп. файлы, сделала реализацию здесь (Inline нужен чтобы компилятор не ругался
на несколько определений функции)
*/
inline void zmq_send_message(Node& node, const Message& msg) {
    zmq_msg_t message;
    zmq_msg_init_size(&message, sizeof(msg));
    std::memcpy(zmq_msg_data(&message), &msg, sizeof(msg));
    zmq_msg_send(&message, node.socket, ZMQ_DONTWAIT);
    zmq_msg_close(&message);
}

inline Message zmq_receive_message(Node& node) {
    zmq_msg_t message;
    zmq_msg_init(&message);

    if (zmq_msg_recv(&message, node.socket, ZMQ_DONTWAIT) == -1) {
        zmq_msg_close(&message);
        return Message("none", -1, -1);
    }

    Message msg;
    std::memcpy(&msg, zmq_msg_data(&message), sizeof(Message));
    zmq_msg_close(&message);
    return msg;
}

