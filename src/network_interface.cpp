#include "network_interface.hpp"
#include <iostream>
#include <thread>
#include <mutex>
#include <unistd.h>

#ifdef _WIN32
#include <Ws2tcpip.h>
#include <winsock.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

static constexpr unsigned int DEFAULT_PORT = 8555;
int server_socket = -1;

std::mutex mtx;

bool try_connect(std::string ip) {
    std::cout << "Connecting to " << ip << "...\n";
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DEFAULT_PORT);
    inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr);

    if (connect(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr))) {
        std::cout << "Could not connect to " << ip << "\n";
        return false;
    }

    std::cout << "Connected to " << ip << "\n";
    return true;
}

void close_connection() {
    if (server_socket != -1) {
#ifdef _WIN32
        closesocket(server_socket);
        WSACleanup();
#else
        close(server_socket);
#endif
        server_socket = -1;
    }
}

void send_message(const message &msg) {
    if (server_socket == -1) {
        std::cout << "Not connected to server\n";
        return;
    }

    char* ser_msg = serialize_message(msg);
    size_t bytes_sent = send(server_socket, ser_msg, sizeof(msg), 0);

    if (bytes_sent == -1) {
#ifdef _WIN32
        std::cout << "Error during send: " << WSAGetLastError() << "\n";
#else
        std::cout << "Error during send: " << strerror(errno) << "\n";
#endif
    }

    delete[] ser_msg;
}

message read_message() {
    if (server_socket == -1) {
        std::cout << "Not connected to server\n";
        return EMPTY_MESSAGE;
    }

    char buffer[sizeof(message)] = {0};
    int bytes_read = recv(server_socket, buffer, sizeof(buffer), 0);

    if (bytes_read > 0) {
        return deserialize_message(buffer);
