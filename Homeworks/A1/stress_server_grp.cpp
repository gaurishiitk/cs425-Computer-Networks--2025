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
#include <atomic>
#include <csignal>

using namespace std;

#define PORT 12345
#define BUFFER_SIZE 1024
#define MAX_GROUPS 1000
#define MAX_GROUP_SIZE 100
#define MAX_CLIENTS 10

std::atomic<int> active_connections = 0;

unordered_map<int, string> clients; // Client socket -> username
unordered_map<string, string> users; // Username -> password
unordered_map<string, unordered_set<int>> groups; // Group -> client sockets
mutex client_mutex, group_mutex;
bool server_running = true; // To handle graceful shutdown

// Utility function to send a message to a specific client
void send_message(int client_socket, const string &message) {
    //handle error
    if(send(client_socket, message.c_str(), message.size(), 0) <= 0) {
        // cout << "Error sending message to client." << endl;
        close(client_socket);
    }
}

// Load users from users.txt
void load_users(const string &filename) {
    ifstream file(filename);
    if (!file) {
        cerr << "Error: Unable to open users.txt" << endl;
        exit(1);
    }

    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        string username, password;
        if (getline(iss, username, ':') && getline(iss, password)) {
            users[username] = password;
        }
    }
}

void handle_broadcast(const string &message, const string &username, int client_socket) {
    string broadcast_msg = message.substr(11);
    if (!broadcast_msg.empty()) {
        broadcast_msg = username + ": " + broadcast_msg;
        lock_guard<mutex> lock(client_mutex);
        for (const auto &[sock, _] : clients) {
            send_message(sock, broadcast_msg);
        }
    }
}

void handle_private_message(const string &message, const string &username, int client_socket) {
    size_t space_pos = message.find(' ', 5);
    if (space_pos != string::npos) {
        string target_user = message.substr(5, space_pos - 5);
        string private_msg = message.substr(space_pos + 1);
        if (!private_msg.empty()) {
            lock_guard<mutex> lock(client_mutex);
            bool user_found = false;
            for (const auto &[sock, user] : clients) {
                if (user == target_user) {
                    send_message(sock, "[Private] " + username + ": " + private_msg);
                    user_found = true;
                    break;
                }
            }
            if (!user_found) {
                send_message(client_socket, "User not found.");
            }
        }
    }
}

void handle_create_group(const string &message, const string &username, int client_socket) {
    if (groups.size() >= MAX_GROUPS) {
        send_message(client_socket, "Maximum number of groups reached.");
        return;
    }

    string group_name = message.substr(14);
    if (!group_name.empty()) {
        lock_guard<mutex> lock(group_mutex);
        if (groups.find(group_name) == groups.end()) {
            groups[group_name] = {client_socket};
            send_message(client_socket, "Group " + group_name + " has been created.");
            string group_created_message = username + " created the group " + group_name + ".";
            for (const auto &[sock, _] : clients) {
                if (sock != client_socket) {
                    send_message(sock, group_created_message);
                }
            }
        } else {
            send_message(client_socket, "Group already exists.");
        }
    }
}

void handle_join_group(const string &message, const string &username, int client_socket) {
    string group_name = message.substr(12);
    if (!group_name.empty()) {
        lock_guard<mutex> lock(group_mutex);
        if (groups.find(group_name) != groups.end()) {
            if (groups[group_name].size() >= MAX_GROUP_SIZE) {
                send_message(client_socket, "Maximum number of members reached in the group.");
                return;
            }
            groups[group_name].insert(client_socket);
            send_message(client_socket, "You joined the group " + group_name + ".");
            string join_group_message = username + " joined the group " + group_name + ".";
            for (int sock : groups[group_name]) {
                if (sock != client_socket) {
                    send_message(sock, join_group_message);
                }
            }
        } else {
            send_message(client_socket, "Group not found.");
        }
    }
}

void handle_group_message(const string &message, const string &username, int client_socket) {
    size_t space_pos = message.find(' ', 11);
    if (space_pos != string::npos) {
        string group_name = message.substr(11, space_pos - 11);
        string group_msg = message.substr(space_pos + 1);
        if (!group_msg.empty()) {
            lock_guard<mutex> lock(group_mutex);
            if (groups.find(group_name) != groups.end() && groups[group_name].find(client_socket) != groups[group_name].end()) {
                for (int sock : groups[group_name]) {
                    send_message(sock, "[Group " + group_name + "] " + username + ": " + group_msg);
                }
            } else {
                send_message(client_socket, "Either Group not found Or you are not in the group.");
            }
        }
    }
}

void handle_leave_group(const string &message, const string &username, int client_socket) {
    string group_name = message.substr(13);
    if (!group_name.empty()) {
        lock_guard<mutex> lock(group_mutex);
        if (groups.find(group_name) != groups.end()) {
            groups[group_name].erase(client_socket);
            send_message(client_socket, "You left the group " + group_name + ".");
            string leave_group_message = username + " left the group " + group_name + ".";
            for (int sock : groups[group_name]) {
                if (sock != client_socket) {
                    send_message(sock, leave_group_message);
                }
            }
        } else {
            send_message(client_socket, "Group not found.");
        }
    } else {
        send_message(client_socket, "Invalid command.");
    }
}

// Handle client connection
void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    string username;

    // Authentication
    send_message(client_socket, "Enter username: ");
    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    username = buffer;

    send_message(client_socket, "Enter password: ");
    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    string password = buffer;

    // Validate credentials
    if (users.find(username) == users.end() || users[username] != password || active_connections >= MAX_CLIENTS) {
        send_message(client_socket, "Authentication failed.");
        // if(active_connections >= MAX_CLIENTS) {
        //     cout<<"Max connections reached. Rejecting client."<<endl;
        // }
        close(client_socket);
        return;
    }

    active_connections++;

    // Add client to the clients map
    {
        lock_guard<mutex> lock(client_mutex);
        clients[client_socket] = username;
    }
    send_message(client_socket, "Welcome to the chat server!\n");

    // Notify the new user about the already active users
    string active_users;
    {
        lock_guard<mutex> lock(client_mutex);
        for (const auto &client : clients) {
            if (client.first != client_socket) {
                active_users += client.second + ", ";
            }
        }
    }
    if (!active_users.empty()) {
        active_users.pop_back(); // Remove the last space
        active_users.pop_back(); // Remove the last comma
        send_message(client_socket, "Active users: " + active_users + "");
    } else {
        send_message(client_socket, "No other users are currently active.");
    }

    // Notify others
    string join_message = username + " has joined the chat."; 
    {
        lock_guard<mutex> lock(client_mutex);
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

        string message(buffer);

        // Parse commands
        if (message.rfind("/broadcast ", 0) == 0) {
            handle_broadcast(message, username, client_socket);
        } else if (message.rfind("/msg ", 0) == 0) {
            handle_private_message(message, username, client_socket);
        } else if (message.rfind("/create_group ", 0) == 0) {
            handle_create_group(message, username, client_socket);
        } else if (message.rfind("/join_group ", 0) == 0) {
            handle_join_group(message, username, client_socket);
        } else if (message.rfind("/group_msg ", 0) == 0) {
            handle_group_message(message, username, client_socket);
        } else if (message.rfind("/leave_group ", 0) == 0) {
            handle_leave_group(message, username, client_socket);
        } else {
            send_message(client_socket, "Invalid command.");
        }
    }
    // Disconnect client
    {
        lock_guard<mutex> lock(client_mutex);
        clients.erase(client_socket);
    }
    close(client_socket);
    active_connections--;

    // Notify others
    string leave_message = username + " has left the chat.";
    {
        lock_guard<mutex> lock(client_mutex);
        for (const auto &[sock, _] : clients) {
            send_message(sock, leave_message);
        }
    }
}

// Graceful shutdown handler
void signal_handler(int signal) {
    server_running = false;
    signal = signal; // To suppress the unused variable warning
    cout << "Shutting down the server..." << endl;
}

int main() {
    //signal(SIGINT, signal_handler); // Handle Ctrl+C to shut down the server
    load_users("users.txt");

    //signal(SIGPIPE, SIG_IGN);

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        cerr << "Error creating socket." << endl;
        return 1;
    }

    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;


    /////
    int yes = 1;

    if(setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        std::cout << "[ERROR] setsockopt error";
        exit(1);
    }
    /////

    if (bind(server_socket, (sockaddr*)&server_address, sizeof(server_address)) < 0) {
        cerr << "Error binding socket." << endl;
        return 1;
    }

    //Maximum Number of Clients
    if (listen(server_socket, SOMAXCONN) < 0) {
        cerr << "Error listening on socket." << endl;
        return 1;
    }

    cout << "Server is running on port " << PORT << "..." << endl;

    while (server_running) {
        sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);
        int client_socket = accept(server_socket, (sockaddr*)&client_address, &client_len);
        if (client_socket < 0) {
            if (!server_running) break;
            cerr << "Error accepting connection." << endl;
            continue;
        }
        thread(handle_client, client_socket).detach();
    }

    close(server_socket);
    return 0;
}
