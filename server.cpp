#include <iostream>

#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "FileMap.h"

#define FILE_DIRECTORY "www/"
#define PORT 8000
#define MAX_CLIENTS 30
#define BUFF_SIZE 1024
#define WELCOME_MESSAGE "Welcome to the server!\r\n"

void serverBody(int server, sockaddr* addr, socklen_t addr_len, FileMap files)
{
    fd_set client_set;
    int client_fd[MAX_CLIENTS] = {0};
    int biggest_fd;

    int server_activity;
    int current_fd;
    int bytes_read;

    char buffer[BUFF_SIZE] = {0};
    int i;

    std::string welcome_message = WELCOME_MESSAGE;

    while(true)
    {
        // Clear the socket set
        FD_ZERO(&client_set);

        // Add server socket to set
        FD_SET(server, &client_set);
        biggest_fd = server;

        // Add existing clients back to socket set
        for(i = 0; i < MAX_CLIENTS; i++)
        {
            current_fd = client_fd[i];

            // If current socket is valid, add to list
            if(current_fd > 0)
            {
                FD_SET(current_fd, &client_set);
            }

            // Update biggest_fd if necessary
            if(current_fd > biggest_fd)
            {
                biggest_fd = current_fd;
            }
        }

        // Wait for activity on one of the clients
        // Timeout is NULL so wait indefinitely
        server_activity = select(biggest_fd + 1, &client_set, NULL, NULL, NULL);

        if(server_activity < 0 && errno != EINTR)
        {
            std::cerr << "Select has failed: " << strerror(errno) << std::endl;
            std::cout << "Trying to continue..." << std::endl;
        }

        // If server socket is set, then a new client is joining
        if(FD_ISSET(server, &client_set))
        {
            current_fd = accept(server, addr, &addr_len);

            if(current_fd == -1)
            {
                std::cerr << "Failed to accept new client: " << strerror(errno) << std::endl;
                std::cout << "Trying to continue..." << std::endl;
            }
            else
            {
                send(current_fd, "HTTP/1.1 200 OK\n", 16, 0);
                send(current_fd, "Content-Type: text/html\n", 24, 0);
                send(current_fd, "Connection: close\n", 18, 0);
                send(current_fd, "\n", 1, 0);

                welcome_message = files.getFile("www/index.html");

                if(send(current_fd, welcome_message.c_str(), welcome_message.size(), 0) == -1)
                {
                    std::cerr << "Failed to send message: " << strerror(errno) << std::endl;
                    std::cout << "Trying to continue..." << std::endl;
                }
                else
                {
                    // Add new socket to array of sockets
                    for(i = 0; i < MAX_CLIENTS; i++)
                    {
                        if(client_fd[i] == 0)
                        {
                            client_fd[i] = current_fd;
                            //std::cout << "New client has joined." << std::endl;

                            break;
                        }
                    }
                }
            }
        }

        for(i = 0; i < MAX_CLIENTS; i++)
        {
            current_fd = client_fd[i];

            if(FD_ISSET(current_fd, &client_set))
            {
                // Clear the buffer
                buffer[0] = '\0';

                // Read the message and check number of bytes read
                bytes_read = recv(current_fd, buffer, BUFF_SIZE, 0);

                // If bytes read is 1 recv failed
                if(bytes_read == -1)
                {
                    std::cerr << "Failed to receive message from client " << current_fd << ": " << strerror(errno) << std::endl;
                    // TODO: How should I handle this? Probably just kick the client.
                    // Should I try to send a message to the client when they get kicked here?
                }
                // recv returns 0 if the client has gracefully disconnected.
                else if(bytes_read == 0)
                {
                    std::cout << "Client " << i << " has disconnected." << std::endl;

                    close(current_fd);
                    client_fd[i] = 0;
                }
                // Handle the recv succeed case
                else
                {
                    // Log the message
                    std::cout << buffer << std::endl;
                    client_fd[i] = 0;
                    close(current_fd);
                }
            }
        }
    }
}

int main(void)
{
    // Create server socket
    int server = socket(AF_INET, SOCK_STREAM, 0);

    // socket returns -1 on failure and sets errno
    if(server == -1)
    {
        std::cerr << "Failed to create server socket: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    // Bind the address and port to server
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    socklen_t addr_len = sizeof(addr);

    if(bind(server, (sockaddr*)&addr, addr_len) == -1)
    {
        std::cerr << "Failed to bind address to server: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    // Start listening for clients on the server
    if(listen(server, SOMAXCONN) == -1)
    {
        std::cerr << "Failed to listen for clients: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    FileMap fileMap(FILE_DIRECTORY);

    serverBody(server, (sockaddr*)&addr, addr_len, fileMap);

    close(server);

    return EXIT_SUCCESS;
}