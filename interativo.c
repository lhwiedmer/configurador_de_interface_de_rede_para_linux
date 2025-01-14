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
#define EXIT_ERROR_CONNECTION 3
#define EXIT_ERROR_COMMUNICATION 4

#define CONFIGURE 1
#define SHOW 2
#define END_INTERACTIVE 3
#define END_BOTH 4
#define COMMANDS 5

void start_connection(int * client_fd) {
	*client_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (*client_fd < 0) {
		perror("Erro ao tentar criar o socket\n");
		exit(EXIT_ERROR_SOCKET);
	}

	struct sockaddr_in serv_address;
	serv_address.sin_family = AF_INET;
	serv_address.sin_port = htons(PORT);

	if (inet_pton(AF_INET, "127.0.0.1", &serv_address.sin_addr) <= 0) {
		perror("Endereço de IP invalido ou nao suportado\n"); //Nao eh para acontecer, ja que eh o endereco local padrao
		exit(EXIT_ERROR_INVALID_IP);
	}

	if (connect(*client_fd, (struct sockaddr*)&serv_address, sizeof(serv_address)) < 0) {
		perror("Erro ao tentar se conectar com o servidor\n");
		exit(EXIT_ERROR_CONNECTION);
	}
}

void showCommands() {
	printf("Digite um dos números para realizar um comando:\n"
		"1 - Configurar o endereco IP de uma interface\n"
		"2 - Mostrar na tela o endereco IP de uma interface especificada\n"
		"3 - Terminar o programa interativo\n"
		"4 - Terminar os dois programas\n"
		"5 - Para mostrar os comandos na tela\n");
}

int main() {
	int client_fd;
	start_connection(&client_fd);
	int command = 0;
	showCommands();
	char buffer[PACKET_SIZE];
	memset(buffer, 0, PACKET_SIZE);
	while (1) {
		scanf("%d", &command);
		switch (command) {
		case CONFIGURE:
			break;
		case SHOW:
			break;
		case END_INTERACTIVE:
			close(client_fd);
			return 0;
		case END_BOTH:
			send(client_fd, "END", sizeof("END"), 0);
			read(client_fd, buffer, PACKET_SIZE);
			if (strcmp(buffer, "OK") != 0) {
				printf("Resposta %s eh diferente da esperada, terminando o programa\n", buffer);
				exit(4);
			}
			printf("Resposta OK do aplicador, terminando programa\n");
			close(client_fd);
			return 0;
		case COMMANDS:
			showCommands();
			break;
		default:
			printf("O comando: %d eh invalido, por favor digite um comando valido\n", command);
			break;
		}
	}
}