#include "nodes.h"

void ComputingNode::broadcast_to_children(const Message& message) {
    for (auto& child : children_) {
        zmq_send_message(child, message); // Внутри используется ZeroMQ-функция zmq_msg_send
    }
}

void ComputingNode::run() {
    while (true) {
        // Есть ли сообщения от детей
        for (auto& child : children_) {
            Message message = zmq_receive_message(child);
            if (strcmp(message.command, "none") != 0) {
                zmq_send_message(node_, message); // Внутри используется ZeroMQ-функция zmq_msg_send
            }
        }

        // Есть ли сообщения от родителя
        Message message = zmq_receive_message(node_);
        if (message.receiver_id != node_.id) {
            broadcast_to_children(message); continue; }

        if (strcmp(message.command, "create") == 0) {
            Node child = create_process(message.additional_data);
            children_.push_back(std::move(child));
            zmq_send_message(node_, Message("create", child.id, child.pid));
        }
        else if (strcmp(message.command, "ping") == 0) {
            zmq_send_message(node_, message);
        }
        else if (strcmp(message.command, "execAdd") == 0) {
            dictionary_[std::string(message.key)] = message.additional_data;
            zmq_send_message(node_, message);
        }
        else if (strcmp(message.command, "execFnd") == 0) {
            auto res = dictionary_.find(std::string(message.key));
            if (res != dictionary_.end()) {
                zmq_send_message(node_, Message("execFnd",
                    node_.id, res->second, message.key));
            } else {
                zmq_send_message(node_, Message("execErr",
                    node_.id, -1, message.key));
            }
        }
    }
}


int main(int argc, char* argv[]) {
    try {
        ComputingNode node(std::atoi(argv[1]));
        node.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
