#include "Server.hpp"

Server::Server () {

}

Server::Server (Server const &other) {

}

Server &Server::operator=(Server const &other) {

}

Server::~Server() {

}

void Server::start() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[1024];
    int port = 6667; // Default IRC port

    // Create the server socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return ;
    }

    // Set up the server address struct
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET; // Set the address family to IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY; // Bind to all available interfaces
    server_addr.sin_port = htons(port); // Convert the port to network byte order

    // Bind the socket to the address and port
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_fd);
        return ;
    }

    // Listen for incoming connections
    if (listen(server_fd, 5) < 0) {
        perror("listen");
        close(server_fd);
        return ;
    }

    std::cout << "Server listening on port " << port << std::endl;

    #define MAX_CLIENTS 1024
    struct pollfd fds[MAX_CLIENTS];

    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    int nfds = 1; // Number of active pollfds

    while (1) {
        int activity = poll(fds, nfds, -1);
        if (activity < 0) {
            perror("poll error");
            break;
        }

        // Accept new client
        if (fds[0].revents & POLLIN) {
            int client_fd = accept(server_fd, NULL, NULL);
            if (nfds < MAX_CLIENTS) {
                fds[nfds].fd = client_fd;
                fds[nfds].events = POLLIN;
                nfds++;
                Client *new_client;
                new_client.fd = client_fd;
                _clients.push_back(new_client);
            } else {
                close(client_fd); // Too many clients
            }
        }

        // Handle client data
        for (int i = 1; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                char buffer[1024];
                int bytes = recv(fds[i].fd, buffer, sizeof(buffer), 0);
                if (bytes <= 0) {
                    // Client disconnected or error
                    close(fds[i].fd);
                    fds[i] = fds[nfds - 1];
                    nfds--;
                    i--; // Check swapped index next
                } else {
                    // Process client message
                    buffer[bytes] = 0;
                    handleClientMessage(fds[i].fd, buffer);
                }
            }
        }
    }
}

void Server::handleClientMessage(int client_fd, const char *msg) {
    std::stringstream pass;

    getline(buffer, pass, ' ');
    getline(buffer, pass, ' ');
    if (pass == "PASS") {
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (_clients[i].fd == client_fd)
                _client[i].pass = msg;
        }
    }
}
