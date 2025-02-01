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

unordered_map<int, string> clients; // Client socket -> username
unordered_map<string, string> users; // Username -> password
unordered_map<string, unordered_set<int>> groups; // Group -> client sockets
mutex client_mutex, group_mutex;
bool server_running = true; // To handle graceful shutdown

// Utility function to send a message to a specific client
void send_message(int client_socket, const string &message) {
    send(client_socket, message.c_str(), message.size(), 0);
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
    if (users.find(username) == users.end() || users[username] != password) {
        send_message(client_socket, "Authentication failed.\n");
        close(client_socket);
        return;
    }

    // Add client to the clients map
    {
        lock_guard<mutex> lock(client_mutex);
        clients[client_socket] = username;
    }
    send_message(client_socket, "\nWelcome to the chat server!\n");

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
        send_message(client_socket, "Active users: " + active_users + "\n");
    } else {
        send_message(client_socket, "No other users are currently active.\n");
    }

    // Notify others
    string join_message = username + " has joined the chat.\n"; 
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
            string broadcast_msg = username + ": " + message.substr(11) + "\n";
            lock_guard<mutex> lock(client_mutex);
            for (const auto &[sock, _] : clients) {
                send_message(sock, broadcast_msg);
            }
        } else if (message.rfind("/msg ", 0) == 0) {
            size_t space_pos = message.find(' ', 5);
            if (space_pos != string::npos) {
                string target_user = message.substr(5, space_pos - 5);
                string private_msg = message.substr(space_pos + 1);
                lock_guard<mutex> lock(client_mutex);
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
        } else if (message.rfind("/create_group ", 0) == 0) {//--let everyone know that a group has been created
            string group_name = message.substr(14);
            {
                lock_guard<mutex> lock(group_mutex);
                if (groups.find(group_name) == groups.end()) {
                    groups[group_name] = {client_socket};
                    //notify the user
                    send_message(client_socket, "Group " + group_name + " has been created.\n");
                    //notify others
                    string group_created_message = username + " created the group " + group_name + ".\n";
                    for (const auto &[sock, _] : clients) {
                        if (sock != client_socket) {
                            send_message(sock, group_created_message);
                        }
                    }
                    //
                } else {
                    send_message(client_socket, "Group already exists.\n");
                }
            }
        } else if (message.rfind("/join_group ", 0) == 0) {//--let everyone in the group know that a user has joined that group
            string group_name = message.substr(12);
            {
                lock_guard<mutex> lock(group_mutex);
                if (groups.find(group_name) != groups.end()) {
                    groups[group_name].insert(client_socket);
                    //notify the user
                    send_message(client_socket, "You joined the group " + group_name + ".\n");
                    //notify others in the group
                    string join_group_message = username + " joined the group " + group_name + ".\n";
                    for (int sock : groups[group_name]) {
                        if (sock != client_socket) {
                            send_message(sock, join_group_message);
                        }
                    }
                    //
                } else {
                    send_message(client_socket, "Group not found.\n");
                }
            }
        } else if (message.rfind("/group_msg ", 0) == 0) {
            size_t space_pos = message.find(' ', 11);
            if (space_pos != string::npos) {
                string group_name = message.substr(11, space_pos - 11);
                string group_msg = message.substr(space_pos + 1);
                {
                    lock_guard<mutex> lock(group_mutex);
                    //check if the user is in the requested group or not   
                    //if they are in the group then send the message to all the users in the group
                    //otherwise send a message to the user that they are not in the group or the group does not exist
                    if (groups.find(group_name) != groups.end() && groups[group_name].find(client_socket) != groups[group_name].end()) {
                        for (int sock : groups[group_name]) {
                            send_message(sock, "[Group " + group_name + "] " + username + ": " + group_msg + "\n");
                        }
                    } else {
                        send_message(client_socket, "Either Group not found Or you are not in the group.\n");
                    }
                }
                //     if (groups.find(group_name) != groups.end()) {
                //         for (int sock : groups[group_name]) {
                //             send_message(sock, "[Group " + group_name + "] " + username + ": " + group_msg + "\n");
                //         }
                //     } else {
                //         send_message(client_socket, "Group not found.\n");
                //     }
                // }
            }
        } else if (message.rfind("/leave_group ", 0) == 0) {//--let everyone in the group know that a user has left that group
            string group_name = message.substr(13);
            {
                lock_guard<mutex> lock(group_mutex);
                if (groups.find(group_name) != groups.end()) {
                    groups[group_name].erase(client_socket);
                    //notify the user
                    send_message(client_socket, "You left the group " + group_name + ".\n");
                    //notify others in the group
                    string leave_group_message = username + " left the group " + group_name + ".\n";
                    for (int sock : groups[group_name]) {
                        if (sock != client_socket) {
                            send_message(sock, leave_group_message);
                        }
                    }
                    //
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
        lock_guard<mutex> lock(client_mutex);
        clients.erase(client_socket);
    }
    close(client_socket);

    // Notify others
    string leave_message = username + " has left the chat.\n";
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
    cout << "Shutting down the server..." << endl;
}

int main() {
    signal(SIGINT, signal_handler); // Handle Ctrl+C to shut down the server
    load_users("users.txt");

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
        std::cout << "[ERROR] setsockopt error\n";
        exit(1);
    }
    /////

    if (bind(server_socket, (sockaddr*)&server_address, sizeof(server_address)) < 0) {
        cerr << "Error binding socket." << endl;
        return 1;
    }

    if (listen(server_socket, 10) < 0) {
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
