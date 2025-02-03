#include <iostream>
#include <vector>
#include <cstring>
#include <string>
#include <unistd.h>
#include <thread>
#include <mutex>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024
#define USERNAME "alice"
#define PASSWORD "password123"
#define EXIT "/exit"

void handle_server_messages(int server_socket) {
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(server_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            close(server_socket);
            return;
        }
    }
}

std::pair<int, int> connectClient(int i, float &successful_connections) 
{
    std::pair<int, int> socket_success = {0, 0};

    // Client connect code
    int client_socket;
    sockaddr_in server_address{};

    // Socket creation failure
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        std::cerr << "Error creating socket." << std::endl;
        return socket_success;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(12345);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connecting to the server
    if (connect(client_socket, (sockaddr*)&server_address, sizeof(server_address)) < 0) {
        std::cerr << "Error connecting to server." << std::endl;
        return socket_success;
    }

    // Authentication
    std::string username, password;
    char buffer[BUFFER_SIZE];

    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0); // Receive the message "Enter the user name" for the server
    // You should have a line like this in the server.cpp code: send_message(client_socket, "Enter username: ");
    send(client_socket, USERNAME, strlen(USERNAME), 0);

    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0); // Receive the message "Enter the password" for the server
    send(client_socket, PASSWORD, strlen(PASSWORD), 0);

    memset(buffer, 0, BUFFER_SIZE);
    // Depending on whether the authentication passes or not, receive the message "Authentication Failed" or "Welcome to the server"
    recv(client_socket, buffer, BUFFER_SIZE, 0); 

    // Failed authenticaion
    if (std::string(buffer).find("Authentication failed") != std::string::npos) {
        close(client_socket);
        return socket_success;
    }

    // Successful connection
    socket_success = {client_socket, 1};
    successful_connections++;
    std::cout << "Client " << i << " connected to the server." << std::endl;

    return socket_success;
}

void disconnectClient(int client_id, std::vector< std::pair<int, bool> > &client_status)
{
    if(client_status[client_id].second == 1) {
        send(client_status[client_id].first, EXIT, strlen(EXIT), 0);
        close(client_status[client_id].first);
    }
    return;
}

int main(int argc, char *argv[])
{
    if(argc <= 1) {
        std::cout << "[USE]: a.exe num_clients\n";
        return 1;
    }
    int num_clients = std::stoi(argv[1]);

    // Success rate
    float successful_connections = 0;
    float success_rate = 0;

    std::vector< std::pair<int, bool> > client_status(num_clients, {0, 0});
    // Connect all clients
    for(int i = 0; i < num_clients; i++) {
        client_status[i] = connectClient(i, successful_connections);
    }

    success_rate = successful_connections / num_clients;
    std::cout << "Success: " << successful_connections << "\nSuccess rate: " << success_rate << std::endl;

    // Disconnect all clients
    // for(int i = 0; i < num_clients; i++) {
    //     disconnectClient(i, client_status);
    // }

    return 0;
}