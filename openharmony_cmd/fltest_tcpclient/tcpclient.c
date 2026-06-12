#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8888
#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
	int sock_fd;
	struct sockaddr_in server_addr;
	char buf[BUF_SIZE];
	const char *server_ip = "127.0.0.1";
	const char *message = "Hello from TCP client!";

	if (argc >= 2)
		server_ip = argv[1];
	if (argc >= 3)
		message = argv[2];

	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd < 0) {
		perror("socket");
		return -1;
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);

	if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
		fprintf(stderr, "Invalid address: %s\n", server_ip);
		close(sock_fd);
		return -1;
	}

	printf("Connecting to %s:%d ...\n", server_ip, PORT);

	if (connect(sock_fd, (struct sockaddr *)&server_addr,
		    sizeof(server_addr)) < 0) {
		perror("connect");
		close(sock_fd);
		return -1;
	}

	printf("Connected. Sending: %s\n", message);
	write(sock_fd, message, strlen(message));

	memset(buf, 0, sizeof(buf));
	ssize_t n = read(sock_fd, buf, sizeof(buf) - 1);
	if (n > 0) {
		buf[n] = '\0';
		printf("Received: %s\n", buf);
	}

	close(sock_fd);
	printf("Connection closed.\n");
	return 0;
}
