#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#define PACKET_SIZE 1024
#define PORT 8000

#define EXIT_ERROR_SOCKET 1
#define EXIT_ERROR_BINDING 2
#define EXIT_ERROR_COMMUNICATION 3
#define EXIT_ERROR_SOCKET_CONFIG 4
#define EXIT_ERROR_LISTEN 5
#define EXIT_ERROR_ACCEPT 6

#define CONFIGURE '1'
#define SHOW '2'
#define END_PROGRAM '3'

//Configura o IP e a subnet mask de uma interface rede, retorna 0 no caso de sucesso e 1, 2 ou 3 para erros
int configInterface(char* interface, char* IPaddr, char* subnetMask) {
	int sockfd;
    struct ifreq ifr;
    struct sockaddr_in *addr;

	//Tenta criar o socket, termina o programa no caso de falha
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
		fprintf(stderr, "Erro ao tentar criar o socket\n");
		exit(EXIT_ERROR_SOCKET);
    }

	//Copia o nome da interface
	strncpy(ifr.ifr_name, interface, IFNAMSIZ);

	//Verifica se a interface está ativa, se nao estiver retorna 1
	if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0) {
        close(sockfd);
        return 1;
    }
  
	addr = (struct sockaddr_in *)&ifr.ifr_addr; //Passa o addr da interface
	addr->sin_family = AF_INET; //Seta como IPv4
	inet_pton(AF_INET, IPaddr, &addr->sin_addr); //Escreve a estrutura de endereco traduzida em sin_addr

	//Tenta configurar o IP da interface, se nao conseguir retorna 2
	if (ioctl(sockfd, SIOCSIFADDR, &ifr) < 0) {
        close(sockfd);
        return 2;
    }

    addr = (struct sockaddr_in *)&ifr.ifr_netmask; //Passa o subnetMask da interface
    addr->sin_family = AF_INET; //Seta como IPv4
    inet_pton(AF_INET, subnetMask, &addr->sin_addr); //Escreve a estrutura de endereco traduzida em sin_addr  

	//Tenta configurar a subnetMask da interface, se nao conseguir retorna 3
    if (ioctl(sockfd, SIOCSIFNETMASK, &ifr) < 0) {
        close(sockfd);
        return 3;
    }

	//Se a configuracao der certo retorna 0
    close(sockfd);
    return 0;
}

//Separa a mensagem presente em buffer e realiza a configuracao da interface, depois manda o resultado para o programa interativo
void configure(int new_socket, char* buffer) {
	int i = 2;
	int j = 0;

	//Le a interface presente no buffer
	char interface[50];
	for (j = 0; buffer[i] != ']' && buffer[i] != '\0'; i++, j++) {
		interface[j] = buffer[i];
	}
	interface[j] = '\0';
	i++;

	//Le o endereco de IP presente no buffer
	char IPaddr[20];
	for (j = 0; buffer[i] != ']' && buffer[i] != '\0'; i++, j++) {
		IPaddr[j] = buffer[i];
	}
	IPaddr[j] = '\0';
	i++;

	//Le a subnet mask presente no buffer
	char subnetMask[20];
	for (j = 0; buffer[i] != '\0'; i++, j++) {
		subnetMask[j] = buffer[i];
	}
	subnetMask[j] = '\0';

	//Tenta configurar a interface
	int result = configInterface(interface, IPaddr, subnetMask);
	char asw[2];
	sprintf(asw, "%d", result);

	//Tenta mandar o resultado para o programa interativo, termina o programa no caso de falha
	if (send(new_socket, asw, 2, 0) == -1) {
		fprintf(stderr, "Erro ao tentar enviar mensagem, terminando programa\n");
		exit(EXIT_ERROR_COMMUNICATION);
	}
}

//Escreve uma mensagem formata em ret com o IP e com a subnet mask da interface requisitada, retorna 0 no caso de sucesso e 1, 2 no caso de falha
int getInterfaceIP(char* iface, char* ret) {
	int sockfd;
	struct ifreq ifr;
	
	//Tenta criar o socket, termina o programa no caso de falha
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		fprintf(stderr, "Erro ao tentar criar socket\n");
		exit(1);
	}

	ifr.ifr_addr.sa_family = AF_INET;//Seta como IPv4

	//Copia o nome da interface
	strncpy(ifr.ifr_name , iface , IFNAMSIZ-1);

	//Verifica se a interface está ativa, se nao estiver retorna 1
	if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0) {
		close(sockfd);
        return 1;
    }

	//Tenta pegar o IP da interface, se nao conseguir retorna 2
	if (ioctl(sockfd, SIOCGIFADDR, &ifr) < 0) {
		close(sockfd);
		return 2;
	}
	char IPaddr[20];
	sprintf(IPaddr, "%s", inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr));

	//Tenta pegar o subnet mask da interface, se nao conseguir retorna 3
    if (ioctl(sockfd, SIOCGIFNETMASK, &ifr) < 0) {
        close(sockfd);
        return 3;
    }
    struct sockaddr_in* subnetMask = (struct sockaddr_in *)&ifr.ifr_netmask;

    uint32_t mask = ntohl(subnetMask->sin_addr.s_addr); //Transforma a subnet mask em ordem de bytes de host
    int prefix = 0;
	//Conta o numero de bits 1 da mascara
    while (mask) {
        prefix += mask & 1;
        mask >>= 1;
    }

	//Monta a mensagem formatada a ser enviada para o programa interativo
	sprintf(ret, "0[Interface: %s, Configured IP:%s/%d", iface, IPaddr, prefix);
	close(sockfd);
	return 0;
}

//Le o nome da interface presente em buffer e manda uma mensagem formatada com os dados dessa interface ou um codigo de erro
void show(int new_socket, char* buffer) {
	//Le a interface presente no buffer
	char interface[50];
	for (int i = 0; buffer[i] != '\0'; i++) {
		interface[i] = buffer[i+2];
	}

	char ret[100];
	int result = getInterfaceIP(interface, ret);

	char result_char = result + '0';
	if (result > 0) { //Tenta mandar um codigo de erro para o programa interativo, termina o program em caso de falha
		char asw[2];
		sprintf(asw, "%c", result_char);
		if (send(new_socket, asw, 2, 0) == -1) {
			fprintf(stderr, "Erro ao tentar enviar mensagem, terminando programa\n");
			exit(EXIT_ERROR_COMMUNICATION);
		}
	} else { //Tenta mandar a resposta formatada para o programa iterativo, termina o programa em caso de falha
		if (send(new_socket, ret, strlen(ret), 0) == -1) {
			fprintf(stderr, "Erro ao tentar enviar mensagem, terminando programa\n");
			exit(EXIT_ERROR_COMMUNICATION);
		};
	}
}

//Manda um OK para o programa interativo e fecha os sockets abertos
void endProgram(int new_socket, int server_fd) {
	send(new_socket, "OK", strlen("OK"), 0);
	close(new_socket);
	close(server_fd);
	printf("Fim de programa requisitado pelo programa interativo\n");
}

int main() {
	int server_fd;

	//Tenta criar um socket, termina o programa em caso de falha
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
		fprintf(stderr, "Erro ao tentar criar o socket\n");
		exit(EXIT_ERROR_SOCKET);
	}

	int opt = 1;
	//Tenta configurar o socket, termina o programa em caso de falha
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) { //SO_REUSEADDR permite o reuso de enderecos locais para esse socket
		fprintf(stderr, "Erro ao tentar ao tentar configurar o socket\n");
		exit(EXIT_ERROR_SOCKET_CONFIG);
	}

	struct sockaddr_in address;
	address.sin_family = AF_INET; //Seta como IPv4
    address.sin_addr.s_addr = INADDR_ANY; //Seta o IP como qualquer IP disponivel no host
    address.sin_port = htons(PORT); //htons transforma de ordem de bytes de host para ordem de bytes de rede

	//Tenta fazer vincular o socket a porta 8000, termina o programa em caso de falha
	if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
		fprintf(stderr, "Erro ao tentar vincular o socket ao endereco\n");
		exit(EXIT_ERROR_BINDING);
	}

	//Comeca a escutar por pedidos de conexao, termina o programa em caso de falha
	if (listen(server_fd, 1) < 0) {
		fprintf(stderr, "Erro ao comecar a escutar por pedidos de conexao\n");
		exit(EXIT_ERROR_LISTEN);
	}
	
	printf("Apĺicador esperando por pedido de conexão\n");

	//Tenta aceitar um pedido de conexao e liga-lo a new_socket, termina o programa em caso de falha
	int new_socket;
	socklen_t addrlen = sizeof(address);
	new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
	if (new_socket < 0) {
		fprintf(stderr, "Erro ao tentar aceitar um pedido de conexao\n");
		exit(EXIT_ERROR_ACCEPT);
	}

	char buffer[PACKET_SIZE];
	printf("Aplicador pronto para receber mensagens\n");
	while (1) {
		memset(buffer, 0, PACKET_SIZE);
		if (read(new_socket, buffer, PACKET_SIZE - 1) == -1) {
			fprintf(stderr,"Erro ao tentar receber a mensagem, terminando o programa\n");
			exit(EXIT_ERROR_COMMUNICATION);
		}
		switch (buffer[0]) {
			case CONFIGURE:
				configure(new_socket, buffer);
				break;
			case SHOW:
				show(new_socket, buffer);
				break;
			case END_PROGRAM:
				endProgram(new_socket, server_fd);
				return 0;
			default:
				fprintf(stderr, "Comando desconhecido, provável problema de comunicação, terminando o programa\n");
				exit(EXIT_ERROR_COMMUNICATION);
				break;
		}
	}
}