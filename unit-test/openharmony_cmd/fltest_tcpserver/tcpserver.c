#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#define PORT 8888
#define BUF_SIZE 1024

static int server_fd = -1;

static void signal_handler(int sig)
{
	if (server_fd >= 0)
		close(server_fd);
	printf("\nServer stopped.\n");
	exit(0);
}

int main(void)
{
	int client_fd;
	struct sockaddr_in server_addr, client_addr;
	socklen_t client_len = sizeof(client_addr);
	char buf[BUF_SIZE];
	ssize_t n;

	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
		perror("socket");
		return -1;
	}

	int opt = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT);

	if (bind(server_fd, (struct sockaddr *)&server_addr,
		 sizeof(server_addr)) < 0) {
		perror("bind");
		close(server_fd);
		return -1;
	}

	if (listen(server_fd, 5) < 0) {
		perror("listen");
		close(server_fd);
		return -1;
	}

	printf("TCP Server listening on port %d\n", PORT);

	while (1) {
		client_fd = accept(server_fd,
				   (struct sockaddr *)&client_addr,
				   &client_len);
		if (client_fd < 0) {
			perror("accept");
			continue;
		}

		printf("Client connected: %s:%d\n",
		       inet_ntoa(client_addr.sin_addr),
		       ntohs(client_addr.sin_port));

		while ((n = read(client_fd, buf, sizeof(buf) - 1)) > 0) {
			buf[n] = '\0';
			printf("Received: %s", buf);
			write(client_fd, buf, n);
		}

		printf("Client disconnected.\n");
		close(client_fd);
	}

	close(server_fd);
	return 0;
}
