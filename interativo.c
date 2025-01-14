#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8000
#define EXIT_ERROR_SOCKET 1
#define EXIT_ERROR_INVALID_IP 2
#define EXIT_ERROR_CONNECTION 3
#define CONFIGURE 1
#define SHOW 2
#define END 3

int main() {
	int client_fd;
	struct sockaddr_in serv_address;
	client_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (client_fd < 0) {
		perror("Erro ao tentar criar o socket\n");
		exit(EXIT_ERROR_SOCKET);
	}

	serv_address.sin_family = AF_INET;
	serv_address.sin_port = htons(PORT);

	if (inet_pton(AF_INET, "127.0.0.1", &serv_address.sin_addr) <= 0) {
		perror("Endereço de IP invalido ou nao suportado\n"); //Nao eh para acontecer, ja que eh o endereco local padrao
		exit(EXIT_ERROR_INVALID_IP);
	}

	if (connect(client_fd, (struct sockaddr*)&serv_address, sizeof(serv_address)) < 0) {
		perror("Erro ao tentar se conectar com o servidor\n");
		exit(EXIT_ERROR_CONNECTION);
	}
	int command = 0;
	while (1) {
		printf("Digite um dos números para realizar um comando:\n"
				"1 - Configurar o endereco IP de uma interface\n"
				"2 - Mostrar na tela o endereco IP de uma interface especificada\n"
				"3 - Terminar o programa\n");
		scanf("%d", &command);
		switch (command) {
		case CONFIGURE:
			break;
		case SHOW:
			break;
		case END:
			break;
		default:
			printf("O comando: %d eh invalido, por favor digite um comando valido\n", command);
			break;
		}
	}

	//Enviar mensagem para o server desligar também
	return 0;
}