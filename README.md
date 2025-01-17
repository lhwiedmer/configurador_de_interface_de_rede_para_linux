Configurador de Interface

Aviso: Essa aplicação foi desenvolvida para Linux OS, e não provavelmente não funcionará em outros sistemas operacionais.

1. Introdução

	A aplicação tem a função de alterar o IP e a mascára de sub-rede de uma interface de rede especificada. A interação com o usuário é feita pelo programa "interativo.c", ao passo que a configuração em si é feita pelo "aplicador.c".

2. Método de Uso

	Para compilar a aplicação, faça: "gcc interativo.c -o interativo" e "gcc aplicador.c -o aplicador".
	Após isso, abra dois terminais Linux, e estando no diretório com o programa, digite "sudo ./aplicador" e siga as instruções,
	no outro terminal faça "./interativo" e siga as instruções. É importante que o aplicador seja usado com sudo, ou o programa
	não poderá configurar nenhuma interface por conta de falta de permissões.
	
	Aviso: O programa só modifica interfaces ativas, então veja as interfaces ativas no seu sistema com o comando "ifconfig" no terminal, ou na aba de rede.

3. Códigos de erro

	O programa interativo pode terminar abruptamente retornado esses códigos:
		1 - Para falha ao tentar criar um socket;
		2 - Para falha ao tentar se conectar com aplicador;
 		3 - Para falha em uma função de send, read ou se uma resposta inesperada for recebida.
	O programa aplicador pode terminar abruptamente retornado esses códigos:
		1 - Para falha ao tentar criar um socket;
		2 - Para falha ao tentar vincular o socket ao endereço;
 		3 - Para falha em uma função de send, read ou se uma resposta inesperada for recebida;
		4 - Para falha ao tentar configurar um socket;
		5 - Para falha ao tentar começar a escutar por pedidos de conexão;
		6 - Para falha ao tentar aceitar um pedido de conexão.

4. Implentação

	A comunicação entre os dois programas é feita por meio de um socket do tipo SOCK_STREAM(TCP). Essa escolha foi feita para garantir confiabilidade 
	da comunicação e evitar problemas que podem acontecer ao usar sockets SOCK_DGRAM(UDP), como perda de pacotes, chegada de pacotes fora de ordem, 
	entre outros problemas que teriam que ser tratados no próprio código.

	Já para a alteração e leitura de informações foi usada a função ioctl(), vinda de sys/ioctl.h. Essa função faz uma syscall que modifica
	arquivos especiais, e por isso é necessário utilizar "sudo" ao utilizar o aplicador.

5. Possíveis melhorias

	Adição de uma interface gráfica que permita que o usuário escolha um comando com o mouse, e digite os parâmetros em campos específicos,
	simplificando o uso em relação à interação com o terminal.
	