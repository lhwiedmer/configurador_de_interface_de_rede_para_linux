#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>

#define PACKET_SIZE 1024
#define PORT 8000

#define EXIT_ERROR_SOCKET 1
#define EXIT_ERROR_INVALID_IP 2
#define EXIT_ERROR_BINDING 3
#define EXIT_ERROR_COMMUNICATION 4
#define EXIT_ERROR_SOCKET_CONFIG 5
#define EXIT_ERROR_LISTEN 6
#define EXIT_ERROR_ACCEPT 7

#define CONFIGURE '1'
#define SHOW '2'
#define END '3'

int configInterface(char* interface, char* IPaddr, char* subnetMask) {
	int sockfd;
    struct ifreq ifr;
    struct sockaddr_in *addr;

	//Cria o socket
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
		fprintf(stderr, "Erro ao tentar criar o socket\n");
		exit(EXIT_ERROR_SOCKET);
    }

	//Copia o nome da interface
	strncpy(ifr.ifr_name, interface, IFNAMSIZ);
	//
	if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0) {
        fprintf(stderr, "Interface inativa\n");
        return 1;
    }
  
	addr = (struct sockaddr_in *)&ifr.ifr_addr;
	addr->sin_family = AF_INET;
	inet_pton(AF_INET, IPaddr, &addr->sin_addr);

	if (ioctl(sockfd, SIOCSIFADDR, &ifr) < 0) {
        printf("Erro ao configurar endereço IP\n");
        close(sockfd);
        return 2;
    }

    addr = (struct sockaddr_in *)&ifr.ifr_netmask;
    addr->sin_family = AF_INET;
    if (inet_pton(AF_INET, subnetMask, &addr->sin_addr));

    if (ioctl(sockfd, SIOCSIFNETMASK, &ifr) < 0) {
        printf("Erro ao configurar máscara de sub-rede\n");
        close(sockfd);
        return 3;
    }

    close(sockfd);
    printf("Configuração da interface %s realizada com sucesso!\n", interface);
    return 0;
}

void configure(int new_socket, char* buffer) {
	int i = 2;
	int j = 0;
	char interface[50];
	for (j = 0; buffer[i] != ']' && buffer[i] != '\0'; i++, j++) {
		interface[j] = buffer[i];
	}
	interface[j] = '\0';
	i++;
	char IPaddr[20];
	for (j = 0; buffer[i] != ']' && buffer[i] != '\0'; i++, j++) {
		IPaddr[j] = buffer[i];
	}
	i++;
	IPaddr[j] = '\0';
	char subnetMask[20];
	for (j = 0; buffer[i] != '\0'; i++, j++) {
		subnetMask[j] = buffer[i];
	}
	subnetMask[j] = '\0';
	printf("Nome: %s\n", interface);
	printf("IP: %s\n", IPaddr);
	printf("SubnetMask: %s\n", subnetMask);
	int result = configInterface(interface, IPaddr, subnetMask);
	char asw[2];
	sprintf(asw, "%d", result);
	if (send(new_socket, asw, 2, 0) == -1) {
		fprintf(stderr, "Erro ao tentar enviar mensagem, terminando programa\n");
		exit(EXIT_ERROR_COMMUNICATION);
	}
	printf("Resposta enviada\n");
}

int getInterfaceIP(char* iface, char* ret) {
	int sockfd;
	struct ifreq ifr;
	
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	ifr.ifr_addr.sa_family = AF_INET;

	strncpy(ifr.ifr_name , iface , IFNAMSIZ-1);

	if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0) {
        fprintf(stderr, "Interface inativa\n");
        return 1;
    }

	ioctl(sockfd, SIOCGIFADDR, &ifr);
	char IPaddr[20];
	sprintf(IPaddr, "%s", inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr));

    if (ioctl(sockfd, SIOCGIFNETMASK, &ifr) < 0) {
        perror("Failed to get subnet mask");
        close(sockfd);
        return 2;
    }
    struct sockaddr_in* subnetMask = (struct sockaddr_in *)&ifr.ifr_netmask;

    uint32_t mask = ntohl(subnetMask->sin_addr.s_addr);
    int prefix = 0;
    while (mask) {
        prefix += mask & 1;
        mask >>= 1;
    }

    printf("%s - %s/%d\n", iface, IPaddr, prefix);
	sprintf(ret, "0]Interface: %s, Configured IP:%s/%d", iface, IPaddr, prefix);
	close(sockfd);
	return 0;
}

void show(int new_socket, char* buffer) {
	char interface[50];
	for (int i = 0; buffer[i] != '\0'; i++) {
		interface[i] = buffer[i+2];
	}
	printf("Nome: %s\n", interface);
	char ret[100];
	int result = getInterfaceIP(interface, ret);
	if (result > 0) {
		char asw[2];
		sprintf(asw, "%d", result);
		send(new_socket, asw, 2, 0);
	} else {
		send(new_socket, "SHOW OK", sizeof("SHOW OK"), 0);
	}
}

void end(int new_socket, int server_fd) {
	send(new_socket, "OK", strlen("OK"), 0);
	printf("Mensagem de OK enviada\n");
	close(new_socket);
	close(server_fd);
	printf("Fim de programa requisitado pelo client\n");
}

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
	printf("Servidor pronto para receber mensagens\n");
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
			case END:
				end(new_socket, server_fd);
				return 0;
			default:
				break;
		}
	}
}