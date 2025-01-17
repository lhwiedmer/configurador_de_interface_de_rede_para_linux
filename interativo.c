#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PACKET_SIZE 1024
#define PORT 8000

#define EXIT_ERROR_SOCKET 1
#define EXIT_ERROR_CONNECTION 2
#define EXIT_ERROR_COMMUNICATION 3

#define CONFIGURE 1
#define SHOW 2
#define END_PROGRAM 3

// Padrao dos pacotes: x]conteudo //Sendo x: 1 para configure, 2 para show e 3 para end

//Inicia conexao com o programa aplicador
void start_connection(int * client_socket) {

	//Tenta criar o socket, termina o programa em caso de falha
	*client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (*client_socket < 0) {
		fprintf(stderr, "Erro ao tentar criar o socket\n");
		exit(EXIT_ERROR_SOCKET);
	}

	struct sockaddr_in serv_address;
	serv_address.sin_family = AF_INET; //Seta como IPv4
	serv_address.sin_port = htons(PORT); //Traduz a porta para ordem de bytes de rede


	inet_pton(AF_INET, "127.0.0.1", &serv_address.sin_addr); //Escreve a estrutura de endereco traduzida em sin_addr  

	//Tenta se conectar ao aplicador, termina o programa em caso de falha
	if (connect(*client_socket, (struct sockaddr*)&serv_address, sizeof(serv_address)) < 0) {
		fprintf(stderr, "Erro ao tentar se conectar com o servidor\n");
		exit(EXIT_ERROR_CONNECTION);
	}
}

//Verifica se uma subnetMask eh válida, se for retorna 1, se não retorna 0
int verifySubnetMask(char* subnetMask) {
	struct in_addr addr;

	//Verifica se está no formato #.#.#.#
    if (inet_pton(AF_INET, subnetMask, &addr) != 1) {
        return 0;
    }

	//Verifica se existe um bit 0 antes de um bit 1
    uint32_t mask = ntohl(addr.s_addr);
    int foundZero = 0;
    for (int i = 31; i >= 0; i--) {
        if ((mask & (1 << i)) == 0) {
            foundZero = 1;
        } else if (foundZero) {
            return 0;
        }
    }

    return 1;
}

void configure(int client_socket, char* buffer) {
	printf("Informe o nome da interface de rede: ");
	char interface[50];
	scanf("%s", interface);

	printf("Informe o novo endereco IP da rede: ");
	char IPaddr[20];
	scanf("%s", IPaddr);
    struct in_addr addr;

	while(!inet_pton(AF_INET, IPaddr, &addr)) { //Verifica se o IP eh válido
		fprintf(stderr, "IP invalido, insira um IP valido: ");
		memset(IPaddr, 0, sizeof(IPaddr));
		scanf("%s", IPaddr);
	}

	printf("Informe a nova mascara de sub-rede: ");
	char subnetMask[20];
	scanf("%s", subnetMask);
	while(!verifySubnetMask(subnetMask)) {
		fprintf(stderr, "Mascara invalida, insira uma mascara valida: ");
		memset(subnetMask, 0, sizeof(subnetMask));
		scanf("%s", subnetMask);
	}

	sprintf(buffer, "1]%s]%s]%s", interface, IPaddr, subnetMask);
	
	//Tenta enviar a interface, o IP e a subnetMask para o aplicador, termina o programa em caso de falha
	if (send(client_socket, buffer, strlen(buffer), 0) == -1) {
		fprintf(stderr,"Erro ao tentar enviar a mensagem, terminando o programa\n");
		exit(EXIT_ERROR_COMMUNICATION);
	}
	printf("Mensagem enviada\n");

	//Tenta ler uma mensagem do aplicador, termina o programa em caso de falha
	if (read(client_socket, buffer, PACKET_SIZE - 1) == -1) {
		fprintf(stderr,"Erro ao tentar receber a mensagem, terminando o programa\n");
		exit(EXIT_ERROR_COMMUNICATION);
	}

	//Testa o codigo da resposta e mostra o resultado para o usuário
	if (buffer[0] == '0') {
		printf("Configuração realizada com sucesso\n");
	} else if (buffer[0] == '1') {
		fprintf(stderr, "Interface inativa\n");
	} else if (buffer[0] == '2') {
		fprintf(stderr, "Falhou ao tentar configurar o endereço IP\n");
	} else if (buffer[0] == '3') {
		fprintf(stderr, "Falhou ao tentar configurar a máscara de subrede\n");
	} else {
		fprintf(stderr, "Erro %c nao eh conhecido, provável problema na comunicação comunicação, terminando programa\n", buffer[0]);
		exit(EXIT_ERROR_COMMUNICATION);
	}
}

void show(int client_socket, char* buffer) {
	printf("Informe o nome da interface de rede: ");
	char interface[50];
	scanf("%s", interface);
	sprintf(buffer, "2]%s", interface);

	//Tenta enviar a interface para o aplicador, termina o programa em caso de falha
	if (send(client_socket, buffer, strlen(buffer), 0) == -1) {
		fprintf(stderr,"Erro ao tentar enviar a mensagem, terminando o programa\n");
		exit(EXIT_ERROR_COMMUNICATION);
	}
	printf("Mensagem enviada\n");

	//Tenta ler uma mensagem do aplicador, termina o programa em caso de falha
	if (read(client_socket, buffer, PACKET_SIZE - 1) == -1) {
		fprintf(stderr,"Erro ao tentar receber a mensagem, terminando o programa\n");
		exit(EXIT_ERROR_COMMUNICATION);
	}

	//Testa o codigo da resposta e mostra o resultado para o usuário
	if (buffer[0] == '0') {
		printf("%s\n", &buffer[2]);
	} else if (buffer[0] == '1') {
		fprintf(stderr, "Interface inativa\n");
	} else if (buffer[0] == '2') {
		fprintf(stderr, "Falhou ao tentar pegar o IP da interface\n");
	} else if (buffer[0] == '3') {
		fprintf(stderr, "Falhou ao tentar pegar a máscara de subrede da interface\n");
	} else {
		fprintf(stderr, "Erro %c nao eh conhecido\n", buffer[0]);
	}
}

void endProgram(int client_socket, char* buffer) {
	//Tenta enviar mensagem sinalizando fim de programa, termina o programa em caso de falha
	if (send(client_socket, "3", sizeof("3"), 0) == -1) {
		fprintf(stderr,"Erro ao tentar enviar a mensagem, terminando o programa\n");
		exit(EXIT_ERROR_COMMUNICATION);
	}
	printf("Mensagem enviada\n");

	//Tenta ler uma mensagem do aplicador, termina o programa em caso de falha
	if (read(client_socket, buffer, PACKET_SIZE - 1) == -1) {
		fprintf(stderr,"Erro ao tentar receber a mensagem, terminando o programa\n");
		exit(EXIT_ERROR_COMMUNICATION);
	}
	//Verifica se a resposta foi a esperada, termina o programa em caso de nao ser
	if (strcmp(buffer, "OK") != 0) {
		fprintf(stderr, "Resposta %s eh diferente da esperada, terminando o programa\n", buffer);
		exit(EXIT_ERROR_COMMUNICATION);
	}
	printf("Resposta OK do aplicador, terminando programa\n");
	close(client_socket);
}

//Mostra os comandos possíveis na tela
void commands() {
	printf("Digite um dos números para realizar um comando:\n"
		"1 - Configurar o endereco IP de uma interface\n"
		"2 - Mostrar na tela o endereco IP de uma interface especificada\n"
		"3 - Terminar o programa\n");
}

int main() {
	int client_socket;
	start_connection(&client_socket);
	int command = 0;
	char buffer[PACKET_SIZE];
	while (1) {
		memset(buffer, 0, PACKET_SIZE);
		commands();
		scanf("%d", &command);
		switch (command) {
		case CONFIGURE:
			configure(client_socket, buffer);
			break;
		case SHOW:
			show(client_socket, buffer);
			break;
		case END_PROGRAM:
			endProgram(client_socket, buffer);
			return 0;
		default:
			printf("O comando: %d eh invalido, por favor digite um comando valido\n", command);
			break;
		}
	}
}