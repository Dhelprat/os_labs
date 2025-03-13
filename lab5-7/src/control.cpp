#include "unordered_set"
#include "nodes.h"
#include <unistd.h>

class Controller {
private:
    std::unordered_set<int> node_ids_;
    std::list<Message> pending_messages_;
    std::list<Node> children_;

    bool input_available() {
        timeval tv{0, 0};
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv);
        return FD_ISSET(STDIN_FILENO, &fds);
    }

    void handle_child_message(const Message& message) {
        if (strcmp(message.command, "create") == 0) {
            node_ids_.insert(message.receiver_id);
            std::cout << "[OK] Created node: " << message.additional_data << std::endl;
        }
        else if (strcmp(message.command, "ping") == 0) {
            std::cout << "[OK] Node: " << message.receiver_id << " is available" << std::endl;
        }
        else if (strcmp(message.command, "execErr") == 0) {
            std::cout << "[OK] " << message.key << " in node " << message.receiver_id << " not found";
        }
        else if (strcmp(message.command, "execAdd") == 0) {
            std::cout << "[OK] " << "Added key: " << message.key << " to node: " << message.receiver_id << std::endl;
        }
        else if (strcmp(message.command, "execFnd") == 0) {
            std::cout << "[OK] Found value: " << message.additional_data << " for key: "
            << message.key << " in node: " << message.receiver_id  << std::endl;
        }

        remove_pending_message(message.command, message.receiver_id);
    }

    void check_pending_messages() {
        auto now_time = std::chrono::system_clock::now();
        auto it = pending_messages_.begin();
        while (it != pending_messages_.end()) {
            if (std::chrono::duration_cast<std::chrono::seconds>(now_time - it->sent_time).count() > 5) {
                std::cout << "[ERROR] Node " << it->receiver_id << " is unavailable" << std::endl;
                it = pending_messages_.erase(it);
            } else {
                ++it;
            }
        }
    }

    void remove_pending_message(const char* cmd, int id) {
        for (auto it = pending_messages_.begin(); it != pending_messages_.end(); ++it) {
            const Message& mes = *it;

            if ((strcmp(mes.command, cmd) == 0 && mes.receiver_id == id) ||
                (strcmp(mes.command, cmd) == 0 && strcmp(mes.command, "create") == 0 && mes.additional_data == id)) {
                pending_messages_.erase(it);
                break; }
        }
    }

    void broadcast_message(const Message& message) {
        pending_messages_.push_back(message);
        for (auto& child : children_) {
            zmq_send_message(child, message); // Внутри используется ZeroMQ-функция zmq_msg_send
        }
    }


public:
    Controller() {
        node_ids_.insert(-1);
    }

    void run() {
        std::string command;
        while (true) {
            // Смотрим, пришли ли сообщения от детей
            for (auto& child : children_) {
                Message message = zmq_receive_message(child);
                if (strcmp(message.command, "none") != 0) {
                    handle_child_message(message);
                }
            }

            check_pending_messages();

            if (!input_available()) { continue; }

            std::cin >> command;
            if (command == "create") {
                int parent_id, child_id;
                std::cin >> child_id >> parent_id;

                if (node_ids_.count(child_id)) {
                    std::cout << "[ERROR] Node with id " << child_id << " already exists" << std::endl; return; }

                if (!node_ids_.count(parent_id)) {
                    std::cout << "[ERROR] Parent with id " << parent_id << " not found" << std::endl; return; }

                if (parent_id == -1) {
                    Node child = create_process(child_id);
                    children_.push_back(std::move(child));
                    node_ids_.insert(child_id);
                    std::cout << "[OK] Created node: " << child.pid << std::endl;
                } else {
                    broadcast_message(Message("create", parent_id, child_id));
                }
            }

            else if (command == "exec") {
                char input[100];
                fgets(input, sizeof(input), stdin);

                int id, val;
                char key[30];

                // По количеству аргументов определяем тип команды: вставка или поиск
                if (sscanf(input, "%d %30s %d", &id, key, &val) == 3) {
                    if (!node_ids_.count(id)) {
                        std::cout << "[ERROR] Node with id " << id << " doesn't exist" << std::endl; return; }

                    broadcast_message(Message("execAdd", id, val, key));
                }
                else if (sscanf(input, "%d %30s", &id, key) == 2) {
                    if (!node_ids_.count(id)) {
                        std::cout << "[ERROR] Node with id " << id << " doesn't exist" << std::endl; return; }

                    broadcast_message(Message("execFnd", id, -1, key));
                }
            }

            else if (command == "ping") {
                int id;
                std::cin >> id;

                if (!node_ids_.count(id)) {
                    std::cout << "[ERROR] Node with id " << id << " doesn't exist" << std::endl;
                } else {
                    broadcast_message(Message("ping", id, 0));
                }
            }

            else if (command == "pingall") {
                for (auto& node_id: node_ids_) {
                    if (node_id == -1) {continue;}

                    broadcast_message(Message("ping", node_id, 0));
                }
            }

            else {
                std::cout << "[ERROR] Command doesn't exist!" << std::endl;
            }
        }
    }
};

int main() {
    try {
        Controller controller;
        controller.run();
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
