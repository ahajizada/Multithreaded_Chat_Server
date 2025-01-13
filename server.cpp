#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <string>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>

const int PORT = 8080;
std::vector<int> clients;
std::mutex clients_mutex;

void handle_client(int client_socket) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            std::cout << "Client disconnected.\n";
            close(client_socket);
            clients_mutex.lock();
            clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end());
            clients_mutex.unlock();
            break;
        }

        std::string message(buffer);
        std::cout << "Received: " << message << "\n";

        // Broadcast message to all clients
        clients_mutex.lock();
        for (int client : clients) {
            if (client != client_socket) {
                send(client, message.c_str(), message.size(), 0);
            }
        }
        clients_mutex.unlock();
    }
}

int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cerr << "Failed to create socket.\n";
        return -1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Failed to bind socket.\n";
        return -1;
    }

    if (listen(server_socket, 10) == -1) {
        std::cerr << "Failed to listen on socket.\n";
        return -1;
    }

    std::cout << "Server started on port " << PORT << ".\n";

    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_len);
        if (client_socket == -1) {
            std::cerr << "Failed to accept connection.\n";
            continue;
        }

        std::cout << "New client connected.\n";

        clients_mutex.lock();
        clients.push_back(client_socket);
        clients_mutex.unlock();

        std::thread client_thread(handle_client, client_socket);
        client_thread.detach();
    }

    close(server_socket);
    return 0;
}
