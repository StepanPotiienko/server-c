#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <WinSock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <unistd.h>
#include <arpa/inet.h>
#endif

#define PORT 8080
#define BUFFER_SIZE 1024

void initialize_winsock()
{
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        fprintf(stderr, "WSAStartup failed. Error Code: %d\n", WSAGetLastError());
        exit(EXIT_FAILURE);
    }

#endif
}

bool isPortTaken(int port)
{
#ifdef _WIN32
    SOCKET sock;
    struct sockaddr_in server;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        printf("Could not create socket: %d.\n", WSAGetLastError());
        return true;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
    {
        closesocket(sock);
        return true;
    }

    closesocket(sock);
    return false;
#else
    return false;
#endif
}

void send_http_response(int client_socket, const char *status, const char *body)
{
    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response),
             "HTTP/1.1 %s\r\n"
             "Content-Type: text/html\r\n"
             "Content-Length: %zu\r\n"
             "\r\n"
             "%s",
             status, strlen(body), body);

    send(client_socket, response, strlen(response), 0);
}

void handle_client(int client_socket)
{
    char buffer[BUFFER_SIZE] = {0};
    // Praceholder
    const char *html_content = "<html><head><title>Welcome</title></head><body><h1>Welcome to the Server</h1></body></html>";

    recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    send_http_response(client_socket, "200 OK", html_content);

    closesocket(client_socket);
}

int main()
{
    initialize_winsock();

    if (isPortTaken(PORT))
    {
        printf("Port %d is already in use. Please choose another one.\n", PORT);
#ifdef _WIN32
        WSACleanup();
#endif
        return -1;
    }

    SOCKET server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        fprintf(stderr, "Socket creation failed. Error Code: %d\n", WSAGetLastError());
#ifdef _WIN32
        WSACleanup();
#endif
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR)
    {
        fprintf(stderr, "Bind failed. Error Code: %d\n", WSAGetLastError());
        closesocket(server_fd);
#ifdef _WIN32
        WSACleanup();
#endif
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) == SOCKET_ERROR)
    {
        fprintf(stderr, "Listen failed. Error Code: %d\n", WSAGetLastError());
        closesocket(server_fd);
#ifdef _WIN32
        WSACleanup();
#endif
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1)
    {
        SOCKET client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (client_socket == INVALID_SOCKET)
        {
            fprintf(stderr, "Accept failed. Error Code: %d\n", WSAGetLastError());
            continue;
        }

        handle_client(client_socket);
    }

    closesocket(server_fd);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
