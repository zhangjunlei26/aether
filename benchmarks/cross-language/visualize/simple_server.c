/**
 * Simple HTTP Server for Benchmark Dashboard
 * Minimal implementation that just works
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

char* read_file(const char* filename, size_t* out_size) {
    FILE* f = fopen(filename, "rb");
    if (!f) return NULL;
    
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char* content = malloc(size + 1);
    fread(content, 1, size, f);
    content[size] = '\0';
    fclose(f);
    
    if (out_size) *out_size = size;
    return content;
}

void send_response(SOCKET client, int status, const char* content_type, const char* body, size_t body_len) {
    char header[512];
    snprintf(header, sizeof(header),
        "HTTP/1.1 %d OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n",
        status, content_type, body_len);
    
    send(client, header, strlen(header), 0);
    if (body && body_len > 0) {
        send(client, body, body_len, 0);
    }
}

int main(int argc, char* argv[]) {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    
    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET) {
        printf("Failed to create socket\n");
        return 1;
    }
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);
    
    if (bind(server, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        printf("Bind failed\n");
        return 1;
    }
    
    if (listen(server, 10) == SOCKET_ERROR) {
        printf("Listen failed\n");
        return 1;
    }
    
    printf("========================================\n");
    printf("  Aether Benchmark Dashboard\n");
    printf("========================================\n");
    printf("\nServer running at http://localhost:8080\n");
    printf("Press Ctrl+C to stop\n\n");
    
    char request[4096];
    
    while (1) {
        SOCKET client = accept(server, NULL, NULL);
        if (client == INVALID_SOCKET) continue;
        
        int received = recv(client, request, sizeof(request) - 1, 0);
        if (received <= 0) {
            closesocket(client);
            continue;
        }
        request[received] = '\0';
        
        // Parse request line
        char method[16], path[256];
        sscanf(request, "%s %s", method, path);
        
        printf("Request: %s %s\n", method, path);
        
        size_t size;
        char* content = NULL;
        const char* content_type = "text/html; charset=utf-8";
        
        if (strcmp(path, "/") == 0 || strcmp(path, "/index.html") == 0) {
            content = read_file("index.html", &size);
        } else if (strcmp(path, "/results.json") == 0) {
            content = read_file("results.json", &size);
            content_type = "application/json";
        } else if (strcmp(path, "/results_ping_pong.json") == 0) {
            content = read_file("results_ping_pong.json", &size);
            content_type = "application/json";
        } else if (strcmp(path, "/results_ring.json") == 0) {
            content = read_file("results_ring.json", &size);
            content_type = "application/json";
        } else if (strcmp(path, "/results_skynet.json") == 0) {
            content = read_file("results_skynet.json", &size);
            content_type = "application/json";
        } else {
            const char* not_found = "404 Not Found";
            send_response(client, 404, "text/plain", not_found, strlen(not_found));
            closesocket(client);
            continue;
        }
        
        if (content) {
            send_response(client, 200, content_type, content, size);
            free(content);
        } else {
            const char* error = "500 File Read Error";
            send_response(client, 500, "text/plain", error, strlen(error));
        }
        
        closesocket(client);
    }
    
    closesocket(server);
    WSACleanup();
    return 0;
}
