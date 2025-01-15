#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PACKET_SIZE 1024
#define PORT 8000

#define EXIT_ERROR_SOCKET 1
#define EXIT_ERROR_INVALID_IP 2
#define EXIT_ERROR_BINDING 3
#define EXIT_ERROR_SOCKET_CONFIG 4
#define EXIT_ERROR_LISTEN 5
#define EXIT_ERROR_ACCEPT 6

#define CONFIGURE 1
#define SHOW 2
#define END_INTERACTIVE 3
#define END_BOTH 4
#define COMMANDS 5


int main() {
	int server_fd;
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
		perror("Erro ao tentar criar o socket\n");
		exit(EXIT_ERROR_SOCKET);
	}

	int opt = 1;
	//SO_REUSEADDR permite o reuso de enderecos locais para esse socket
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
		perror("Erro ao tentar ao tentar configurar o socket\n");
		exit(EXIT_ERROR_SOCKET_CONFIG);
	}

	struct sockaddr_in address;
	address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT); //htons transforma de ordem de bytes de host para ordem de bytes de rede

	if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
		perror("Erro ao tentar vincular o socket ao endereco\n");
		exit(EXIT_ERROR_BINDING);
	}

	if (listen(server_fd, 1) < 0) {
		perror("Erro ao comecar a escutar por pedidos de conexao\n");
		exit(EXIT_ERROR_LISTEN);
	}

	int new_socket;
	socklen_t addrlen = sizeof(address);
	new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
	if (new_socket < 0) {
		perror("Erro ao tentar aceitar um pedido de conexao\n");
		exit(EXIT_ERROR_ACCEPT);
	}

	char buffer[PACKET_SIZE];
	memset(buffer, 0, PACKET_SIZE);

	printf("Servidor pronto para receber mensagens\n");
	int numRead = 0;
	while (1) {
		numRead = read(new_socket, buffer, PACKET_SIZE - 1);
		printf("Buffer: %s\n", buffer);

		if (strcmp(buffer, "END") == 0) {
			send(new_socket, "OK", strlen("OK"), 0);
    		printf("Mensagem de OK enviada\n");

    		close(new_socket);
    		close(server_fd);

			printf("Fim de programa requisitado pelo client\n");
			return 0;
		}
	}
}