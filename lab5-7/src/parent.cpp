#include "nodes.h"
#include <unistd.h> // для работы с системными вызовами

Node::Node(Node&& other) noexcept // Конструктор перемещения
    : id(other.id)
    , pid(other.pid)
    , context(other.context) // указатели
    , socket(other.socket) // (void*)
    , address(std::move(other.address)) // перемещение для экономии памяти (other.address становится пустой строкой)
{
    other.context = nullptr;
    other.socket = nullptr;
}

Node create_node(int id, bool is_child) {
    Node node;
    node.id = id;
    node.pid = getpid();
    int rc;

    node.context = zmq_ctx_new();
    if (!node.context) {
        throw std::runtime_error("Failed to create ZMQ context");
    }

    node.socket = zmq_socket(node.context, ZMQ_DEALER);
    if (!node.socket) {
        zmq_ctx_destroy(node.context);
        throw std::runtime_error("Failed to create ZMQ socket");
    }

    node.address = "tcp://127.0.0.1:" + std::to_string(5555 + id);

    // ребенок подключается к родительскому узлу,
    // а родитель привязывает сокет, чтобы слушать входящие сообщения
    if (is_child) {
        rc = zmq_connect(node.socket, node.address.c_str());
    } else {
        rc = zmq_bind(node.socket, node.address.c_str());
    }

    if (rc != 0) { // rc = -1 или rc = 0 в зависимости от успешности выполнения zmq_conect()/zmq_bind()
        throw std::runtime_error("Failed to " +
            std::string(is_child ? "connect" : "bind") + " ZMQ socket");
    }

    return node;
}

Node create_process(int id) {
    pid_t pid = fork();
    if (pid == 0) { // Если execl() не сработал, то выводим ошибку
        execl("./computing", "computing", std::to_string(id).c_str(), nullptr);
        std::cerr << "execl failed: " << strerror(errno) << std::endl;
        exit(1);
    }
    if (pid == -1) {
        throw std::runtime_error("Fork failed: " +
            std::string(strerror(errno)));
    }

    Node node = create_node(id, false);
    node.pid = pid;
    return node;
}
