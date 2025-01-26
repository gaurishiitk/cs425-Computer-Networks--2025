// Server-side implementation for a multi-threaded chat server

#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
// #include <Winsock2.h>
#include <csignal>

using namespace std;

#define PORT 12345
#define BUFFER_SIZE 1024

std::unordered_map<int, std::string> clients; // Client socket -> username
std::unordered_map<std::string, std::string> users; // Username -> password
std::unordered_map<std::string, std::unordered_set<int>> groups; // Group -> client sockets
std::mutex client_mutex, group_mutex;
bool server_running = true; // To handle graceful shutdown

// Utility function to send a message to a specific client
void send_message(int client_socket, const std::string &message) {
    send(client_socket, message.c_str(), message.size(), 0);
}

// Load users from users.txt
void load_users(const std::string &filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error: Unable to open users.txt" << std::endl;
        exit(1);
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string username, password;
        if (std::getline(iss, username, ':') && std::getline(iss, password)) {
            users[username] = password;
        }
    }
}

// Handle client connection
void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    std::string username;

    // Authentication
    send_message(client_socket, "Enter username: ");
    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    username = buffer;

    send_message(client_socket, "Enter password: ");
    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    std::string password = buffer;

    // Validate credentials
    if (users.find(username) == users.end() || users[username] != password) {
        send_message(client_socket, "Authentication failed\n");
        close(client_socket);
        return;
    }

    // Add client to the clients map
    {
        std::lock_guard<std::mutex> lock(client_mutex);
        clients[client_socket] = username;
    }
    send_message(client_socket, "Welcome to the chat server!\n");

    // Notify others
    std::string join_message = username + " has joined the chat.\n";
    {
        std::lock_guard<std::mutex> lock(client_mutex);
        for (const auto &[sock, _] : clients) {
            if (sock != client_socket) {
                send_message(sock, join_message);
            }
        }
    }

    // Handle commands from the client
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            break;
        }

        std::string message(buffer);

        // Parse commands
        if (message.starts_with("/broadcast ")) {
            std::string broadcast_msg = username + ": " + message.substr(11) + "\n";
            std::lock_guard<std::mutex> lock(client_mutex);
            for (const auto &[sock, _] : clients) {
                send_message(sock, broadcast_msg);
            }
        } else if (message.starts_with("/msg ")) {
            size_t space_pos = message.find(' ', 5);
            if (space_pos != std::string::npos) {
                std::string target_user = message.substr(5, space_pos - 5);
                std::string private_msg = message.substr(space_pos + 1);
                std::lock_guard<std::mutex> lock(client_mutex);
                bool user_found = false;
                for (const auto &[sock, user] : clients) {
                    if (user == target_user) {
                        send_message(sock, "[Private] " + username + ": " + private_msg + "\n");
                        user_found = true;
                        break;
                    }
                }
                if (!user_found) {
                    send_message(client_socket, "User not found.\n");
                }
            }
        } else if (message.starts_with("/create group ")) {
            std::string group_name = message.substr(14);
            {
                std::lock_guard<std::mutex> lock(group_mutex);
                if (groups.find(group_name) == groups.end()) {
                    groups[group_name] = {client_socket};
                    send_message(client_socket, "Group " + group_name + " created.\n");
                } else {
                    send_message(client_socket, "Group already exists.\n");
                }
            }
        } else if (message.starts_with("/join group ")) {
            std::string group_name = message.substr(12);
            {
                std::lock_guard<std::mutex> lock(group_mutex);
                if (groups.find(group_name) != groups.end()) {
                    groups[group_name].insert(client_socket);
                    send_message(client_socket, "You joined the group " + group_name + ".\n");
                } else {
                    send_message(client_socket, "Group not found.\n");
                }
            }
        } else if (message.starts_with("/group msg ")) {
            size_t space_pos = message.find(' ', 11);
            if (space_pos != std::string::npos) {
                std::string group_name = message.substr(11, space_pos - 11);
                std::string group_msg = message.substr(space_pos + 1);
                {
                    std::lock_guard<std::mutex> lock(group_mutex);
                    if (groups.find(group_name) != groups.end()) {
                        for (int sock : groups[group_name]) {
                            send_message(sock, "[Group " + group_name + "] " + username + ": " + group_msg + "\n");
                        }
                    } else {
                        send_message(client_socket, "Group not found.\n");
                    }
                }
            }
        } else if (message.starts_with("/leave group ")) {
            std::string group_name = message.substr(13);
            {
                std::lock_guard<std::mutex> lock(group_mutex);
                if (groups.find(group_name) != groups.end()) {
                    groups[group_name].erase(client_socket);
                    send_message(client_socket, "You left the group " + group_name + ".\n");
                } else {
                    send_message(client_socket, "Group not found.\n");
                }
            }
        } else {
            send_message(client_socket, "Invalid command.\n");
        }
    }

    // Disconnect client
    {
        std::lock_guard<std::mutex> lock(client_mutex);
        clients.erase(client_socket);
    }
    close(client_socket);

    // Notify others
    std::string leave_message = username + " has left the chat.\n";
    {
        std::lock_guard<std::mutex> lock(client_mutex);
        for (const auto &[sock, _] : clients) {
            send_message(sock, leave_message);
        }
    }
}

// Graceful shutdown handler
void signal_handler(int signal) {
    server_running = false;
    std::cout << "Shutting down the server..." << std::endl;
}

int main() {
    signal(SIGINT, signal_handler); // Handle Ctrl+C to shut down the server
    load_users("users.txt");

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Error creating socket." << std::endl;
        return 1;
    }

    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (sockaddr*)&server_address, sizeof(server_address)) < 0) {
        std::cerr << "Error binding socket." << std::endl;
        return 1;
    }

    if (listen(server_socket, 10) < 0) {
        std::cerr << "Error listening on socket." << std::endl;
        return 1;
    }

    std::cout << "Server is running on port " << PORT << "..." << std::endl;

    while (server_running) {
        sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);
        int client_socket = accept(server_socket, (sockaddr*)&client_address, &client_len);
        if (client_socket < 0) {
            if (!server_running) break;
            std::cerr << "Error accepting connection." << std::endl;
            continue;
        }
        std::thread(handle_client, client_socket).detach();
    }

    close(server_socket);
    return 0;
}
