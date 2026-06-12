#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 9000
#define BUFFER_SIZE 1024

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    const char *welcome_message = "Hello from TCP server!\n";

    // 创建socket文件描述符
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 设置socket选项
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // 配置地址结构
    address.sin_family = AF_INET; // 设置地址族为IPv4
    address.sin_addr.s_addr = INADDR_ANY; // 监听所有本地地址
    address.sin_port = htons(PORT);

    // 绑定socket到端口
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 开始监听
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    std::cout << "TCP server listening on port " << PORT << "..." << std::endl;

    while (true) {
        // 接受客户端连接
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // 打印客户端信息
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(address.sin_addr), client_ip, INET_ADDRSTRLEN);
        std::cout << "New connection from " << client_ip << ":" << ntohs(address.sin_port) << std::endl;

        // 发送欢迎消息
        send(new_socket, welcome_message, strlen(welcome_message), 0);
        std::cout << "Welcome message sent" << std::endl;

        // 读取客户端消息
        int valread = read(new_socket, buffer, BUFFER_SIZE);
        std::cout << "Received: " << buffer << std::endl;

        // 发送响应
        const char *response = "Message received!\n";
        send(new_socket, response, strlen(response), 0);
        std::cout << "Response sent" << std::endl;

        // 不关闭客户端连接，保持连接状态
        // close(new_socket);
        // std::cout << "Connection closed" << std::endl;
    }
    // 关闭服务器socket
    close(server_fd);
    std::cout << "Server socket closed" << std::endl;

    return 0;
}
