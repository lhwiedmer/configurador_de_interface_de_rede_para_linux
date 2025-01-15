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
#define END 3
#define COMMANDS 4

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

int isNumber(char c) {
	if ((c < '0') || (c > '9')) {
		return 0;
	}
	return 1;
}

int verifySubnetMask(char* subnetMask) {
	int validBytes[9] = {0, 128, 192, 224, 240, 248, 252, 254, 255};
	int k = 0;
	int j = 0;
	char num[4];
	for (int i = 0; i < 4; i++) {
		for (k = 0; subnetMask[j] != '.' && subnetMask[j] != '\0' && k < 3; j++, k++) {
			if (!isNumber(subnetMask[j])) {
				return 0;
			}
			num[k] = subnetMask[j];
		}
		num[k] = '\0';
		if (((k == 3) && (subnetMask[j] != '.') && (subnetMask[j] != '\0')) || (k == 0)) {
			return 0;
		}
		if ((subnetMask[j] == '\0') && (i != 3)) {
			return 0;
		}
		if ((subnetMask[j] == '.') && (i >= 3)) {
			return 0;
		}
		int n = atoi(num);
		int valid = 0;
		for (int counter = 0; counter < 9; counter++) {
			if (validBytes[counter] == n) {
				valid = 1;
				break;
			}
		}
		if (valid == 0) {
			return 0;
		}
		if (subnetMask[j] == '.') {
			j++;
		}
	}
	return 1;
}


int verifyIP(char* IPaddr) {
	int k = 0;
	int j = 0;
	char num[4];
	for (int i = 0; i < 4; i++) {
		for (k = 0; IPaddr[j] != '.' && IPaddr[j] != '\0' && k < 3; j++, k++) {
			if (!isNumber(IPaddr[j])) {
				return 0;
			}
			num[k] = IPaddr[j];
		}
		num[k] = '\0';
		if (((k == 3) && (IPaddr[j] != '.') && (IPaddr[j] != '\0')) || (k == 0)) {
			return 0;
		}
		if ((IPaddr[j] == '\0') && (i != 3)) {
			return 0;
		}
		if ((IPaddr[j] == '.') && (i >= 3)) {
			return 0;
		}
		int n = atoi(num);
		if ((n > 255) || (n < 0)){
			return 0;
		}
		if (IPaddr[j] == '.') {
			j++;
		}
	}
	return 1;
}

void configure() {
	printf("Informe o nome da interface de rede: ");
	char interface[50];
	scanf("%s", interface);
	printf("Informe o endereco IP da rede: ");
	char IPaddr[20];
	scanf("%s", IPaddr);
	while(!verifyIP(IPaddr)) {
		printf("IP invalido, insira um IP valido: ");
		memset(IPaddr, 0, sizeof(IPaddr));
		scanf("%s", IPaddr);
	}
	printf("Informe a mascara de sub-rede: ");
	char subnetMask[20];
	scanf("%s", subnetMask);
	while(!verifySubnetMask(subnetMask)) {
		printf("Mascara invalida, insira uma mascara valida: ");
		memset(subnetMask, 0, sizeof(subnetMask));
		scanf("%s", subnetMask);
	}
	printf("Dados validos, enviando...\n");
	return;
}

void show() {

}

void end(int client_fd, char* buffer) {
	send(client_fd, "END", sizeof("END"), 0);
	read(client_fd, buffer, PACKET_SIZE - 1);
	if (strcmp(buffer, "OK") != 0) {
		printf("Resposta %s eh diferente da esperada, terminando o programa\n", buffer);
		exit(EXIT_ERROR_COMMUNICATION);
	}
	printf("Resposta OK do aplicador, terminando programa\n");
	close(client_fd);
}

void commands() {
	printf("Digite um dos números para realizar um comando:\n"
		"1 - Configurar o endereco IP de uma interface\n"
		"2 - Mostrar na tela o endereco IP de uma interface especificada\n"
		"3 - Terminar o programa\n"
		"4 - Para mostrar os comandos na tela\n");
}

int main() {
	int client_fd;
	start_connection(&client_fd);
	int command = 0;
	char buffer[PACKET_SIZE];
	while (1) {
		memset(buffer, 0, PACKET_SIZE);
		commands();
		scanf("%d", &command);
		switch (command) {
		case CONFIGURE:
			configure();
			break;
		case SHOW:
			break;
		case END:
			end(client_fd, buffer);
			return 0;
		case COMMANDS:
			commands();
			break;
		default:
			printf("O comando: %d eh invalido, por favor digite um comando valido\n", command);
			break;
		}
	}
}