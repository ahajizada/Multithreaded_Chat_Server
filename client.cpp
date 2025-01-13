#include <iostream>
#include <thread>
#include <string>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>

const int PORT = 8080;

void receive_messages(int socket) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(socket, buffer, sizeof(buffer), 0);
        if (bytes_received > 0) {
            std::cout << "Message: " << buffer << "\n";
        }
    }
}

int main() {
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        std::cerr << "Failed to create socket.\n";
        return -1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (connect(client_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Failed to connect to server.\n";
        return -1;
    }

    std::cout << "Connected to the server.\n";

    std::thread receiver(receive_messages, client_socket);
    receiver.detach();

    while (true) {
        std::string message;
        std::getline(std::cin, message);

        send(client_socket, message.c_str(), message.size(), 0);
    }

    close(client_socket);
    return 0;
}
