## Assignment Features

### How to Run the C++ Chat Server and Client  
1. **Compile the code**  
   - Run:  
     ```sh
     make
     ```
2. **Start the server**  
   - Run:  
     ```sh
     ./server
     ```
3. **Start the client**  
   - If running on the **same PC**, use:  
     ```sh
     ./client
     ```  
   - If running on a **different PC**:  
     1. Find the server's IP using:  
        ```sh
        ifconfig  # or use `ip a`
        ```
     2. Update the client source code with the server's IP.  
     3. Recompile the client.  
     4. Run:  
        ```sh
        ./client
        ```

Now, the client can connect to the server.

### Implemented Features:
- TCP-based server listening on port 12345.
- Supports multiple concurrent client connections.
- User authentication using a `users.txt` file.
- List of active users visible after user authentication
- Private messaging between users (`/msg <username> <message>`).
- Broadcasting messages to all users (`/broadcast <message>`).
- Group creation (`/create_group <group_name>`), joining (`/join_group <group_name>`), leaving (`/leave_group <group_name>`), and messaging (`/group_msg <group_name> <message>`).
- Thread-safe operations using `std::mutex`.
- Proper handling of client disconnections.


### Not Implemented Features:
- No persistent storage for group membership (restarts reset groups).
- No command history or logging of messages.
- No encryption for secure communication.

## Design Decisions

### Threading Model
- The server creates a **new thread per client connection** (`std::thread(handle_client, client_socket).detach();`).
- This ensures each client is handled independently but increases resource usage with many clients.

### Synchronization
- Used `std::mutex` with `std::lock_guard<std::mutex>` for shared resources:
  - `client_mutex` for `clients` map (tracking active users).
  - `group_mutex` for `groups` map (managing group memberships).
- Prevents race conditions when multiple clients access or modify these structures.

### Command Parsing
- Messages are parsed using `std::string` functions.
- Commands start with `/` to distinguish them from normal messages.
- Server extracts command arguments and executes appropriate functions.

## Implementation Details

### High-Level Idea of Important Functions:

1. **`handle_client(int client_socket)`**:
    - Handles communication with an individual client.
    - Authenticates the user by verifying their username and password.
    - Receives and processes user commands (broadcast, private messages, group operations, etc.).
    - Manages client disconnection and notifies other users when a client leaves.
2. **`send_message(int client_socket, const string &message)`**:
    - Sends a message to a specific client.
    - Uses the `send()` system call to transmit the message over the network.
    - Handles errors such as connection loss and ensures proper cleanup if sending fails.
3. **`handle_broadcast(const string &message, const string &username, int client_socket)`**:
    - Extracts the broadcast message from the command input.
    - Iterates through all connected clients and sends the message.
    - Uses a mutex to prevent race conditions while accessing the `clients` list.
4. **`handle_private_message(const string &message, const string &username, int client_socket)`**:
    - Extracts the recipient username and message content from the input.
    - Searches for the recipient in the `clients` map.
    - If the recipient is found, sends the message privately.
    - Notifies the sender if the recipient does not exist.
5. **`handle_create_group(const string &message, const string &username, int client_socket)`**:
    - Checks if the maximum number of groups has been reached.
    - Creates a new group and adds the requesting client as the first member.
    - Notifies all users about the creation of the group.
6. **`handle_join_group(const string &message, const string &username, int client_socket)`**:
    - Extracts the group name from the message.
    - Verifies that the group exists and has not exceeded the maximum member limit.
    - Adds the client to the group's member list and notifies the group.
7. **`handle_group_message(const string &message, const string &username, int client_socket)`**:
    - Ensures that the sender is a member of the specified group.
    - Broadcasts the message to all members of the group.
    - If the sender is not a group member, sends an error response.
8. **`handle_leave_group(const string &message, const string &username, int client_socket)`**:
    - Removes the client from the specified group.
    - Notifies remaining group members about the user's departure.
    - If the group becomes empty, it remains available for new members.
9. **`load_users(const string &filename)`**:
    - Reads the `users.txt` file and loads the username-password pairs into memory.
    - Parses each line, extracting usernames and corresponding passwords.
    - If the file cannot be opened, terminates the server with an error message.
10. **`signal_handler(int signal)`**:
    - Handles server shutdown by setting `server_running` to false.
    - Ensures that all client connections are properly closed before exiting.

### Code Flow
1. **Server Setup:**
   - Loads users from `users.txt`.
   - Binds to `PORT 12345` and listens for connections.
2. **Client Connection:**
   - A new client thread is created.
   - User authentication is performed.
   - Active users are notified of the new client.
3. **Message Handling:**
   - Commands like `/msg`, `/broadcast`, `/group_msg` are parsed and executed.
   - Synchronization ensures safe modification of shared data structures.
4. **Client Disconnection:**
   - User is removed from `clients`.
   - Groups are updated accordingly.
   - Other users are notified.

## Testing

### Correctness Testing
- **Authentication:** Validated user login with correct username/password.
- **Private Messaging:** Verified private messages delivered to the intended recipient.
- **Broadcast Messaging:** Ensured all clients received broadcast messages.
- **Group Management:** Tested creating, joining, and messaging within groups. Verified group limits were respected.

### Stress Testing
- **Concurrency:** Tested with 100+ clients to evaluate server performance.
- **Group Operations:** Stress-tested group creation, joining, and messaging.

### Server Restrictions

- **Maximum Clients:** Successfully tested with up to 2981 simultaneous clients.
- **Maximum Groups:** Limited to 1000 groups.
- **Maximum Group Size:** Each group can have up to 100 members.
- **Maximum Message Size:** Messages are limited to 1024 bytes.

## Challenges and Solutions

### Race Conditions in Shared Data Structures
- **Problem:** Simultaneous client modifications of `clients` and `groups`.
- **Solution:** Used `std::mutex` to ensure thread-safe access.

### Handling Partial Data Reception
- **Problem:** `recv()` might not receive full messages in one go.
- **Solution:** Implemented buffer handling and ensured proper message parsing.

### Debugging Multithreading Issues
- **Problem:** Hard-to-trace bugs due to concurrency.
- **Solution:** Added debug messages and tested in controlled environments.

## Contribution of Team Members

| Member  | Contribution (%) | Role |
|---------|----------------|------|
| Gaurish Bansal (210390)  | 33.33%            | Server design, authentication handling |
|  Madhur Bansal (210572)  | 33.33%            | Message parsing, command execution |
| Lakshika (210554) | 33.33%             | Testing, debugging, and documentation |

## References
- C++ reference documentation for `std::thread`, `std::mutex`
- Linux `man` pages for `socket()`, `bind()`, `listen()`, `accept()`, `recv()`, `send()`

## Declaration
We hereby declare that we did not indulge in any plagiarism while completing this assignment.

## Feedback
- The assignment was well-structured and reinforced important networking concepts.
- The problem statement could clarify whether group memberships should persist across server restarts.
- Testing corner cases (e.g., disconnection during message transmission) required extra effort and could be discussed more in class.



