## Assignment Features

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

### High-Level Overview of Important Functions
- `send_message(int client_socket, const string &message)`: Sends a message to a client.
- `load_users(const string &filename)`: Loads valid usernames and passwords from `users.txt`.
- `handle_client(int client_socket)`: Handles a single client session (authentication, message handling).
- `signal_handler(int signal)`: Handles graceful shutdown of the server.

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
- Connected multiple clients and ensured:
  - Authentication worked correctly.
  - Private messaging was properly delivered.
  - Broadcast messages reached all clients.
  - Group functionalities operated as expected.

### Stress Testing
- Launched 100+ clients to test server performance and thread management.
- Attempted concurrent group creation, joining, and messaging.
- Measured response times and identified minor performance bottlenecks.

## Server Restrictions
- **Maximum Clients:** 2981 clients were connected (in Virtual Machine)
- **Maximum Groups:** 1000.
- **Maximum Group Size:** 100 members.
- **Maximum Message Size:** 1024 bytes per message.

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
| Gaurish Bansal  | 40%            | Server design, authentication handling |
| Lakshika    | 30%            | Message parsing, command execution |
| Madhur Bansal | 30%            | Testing, debugging, and documentation |

## References
- C++ reference documentation for `std::thread`, `std::mutex`
- Linux `man` pages for `socket()`, `bind()`, `listen()`, `accept()`, `recv()`, `send()`

## Declaration
We hereby declare that we did not indulge in any plagiarism while completing this assignment.

## Feedback
- The assignment was well-structured and reinforced important networking concepts.
- The problem statement could clarify whether group memberships should persist across server restarts.
- Testing corner cases (e.g., disconnection during message transmission) required extra effort and could be discussed more in class.



