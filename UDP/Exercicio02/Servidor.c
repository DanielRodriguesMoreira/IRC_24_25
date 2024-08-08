/*============================== Servidor basico UDP ==========================
  Este servidor UDP destina-se a mostrar os conteudos dos datagramas recebidos.
  O porto de escuta encontra-se definido pela constante SERV_UDP_PORT.
  Assume-se que as mensagens recebida sao cadeias de caracteres (ou seja,
  "strings").
===============================================================================*/

#include <stdio.h>
#include <winsock.h>

#pragma comment(lib, "Ws2_32.lib")

#define SERV_UDP_PORT 6000
#define BUFFERSIZE     4096

void Abort(char* msg);

/*________________________________ main ________________________________________
*/

int main(int argc, char* argv[])
{
	SOCKET sockfd;
	int iResult, nbytes, msg_len, length_addr;
	struct sockaddr_in serv_addr, cli_addr;
	char buffer[BUFFERSIZE];
	WSADATA wsaData;

	/*=============== INICIA OS WINSOCKS ==============*/
	
	/*
	* Este é o primeiro passo que tem que ser feito quando se pretende
	* criar uma aplicação recorrendo a Windows Sockets.
	* No fundo estamos a indicar que a nossa aplicação pretende usar
	* Windows Sockets e será retornado um código de erro caso não
	* seja possível.
	*
	* NOTA: esta função não está no formulário que terão acesso
	* no dia do teste e por isso não será alvo de avaliação nos testes
	* práticos da minha turma.
	*
	* NOTA2: falo apenas por mim, não garanto que não apareça em testes
	* de outros professores!
	*/
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		getchar();
		exit(1);
	}

	/*============ CRIA O SOCKET PARA RECEPCAO/ENVIO DE DATAGRAMAS UDP ============*/

	/*
	* Nas linhas seguintes estamos a criar o Socket que será usado na comunicação
	* entre aplicações.
	*/
	if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
		Abort("Impossibilidade de abrir socket");

	/*=============== ASSOCIA O SOCKET AO  ENDERECO DE ESCUTA ===============*/

	/*
	* Nas linhas seguintes estamos a preencher a estrutura, nas respetivas propriedades
	* com os dados que queremos para o socket.
	* sin_family
	* sin_addr.s_addr
	* sin_port
	* 
	* NOTA: notem que a linha 82, onde especificamos o IP, é diferente do que fazemos
	*		do lado do cliente!
	*		Isto porque, do lado do cliente estamos a especificar qual o IP do servidor
	*		a quem queremos enviar um datagram, enquanto que, do lado do servidor,
	*		estamos a especificar que pretendemos receber datagramas de qualquer
	*		interface de rede (daí usarmos a constante INADDR_ANY).
	* 
	* NOTA2: tal como no cliente, aqui também temos que usar funções "auxiliares" para
	*		 converter o IP e porto para algo reconhecido pelos sockets.
	*/
	memset((char*)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET; /*Address Family: Internet*/
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  /*Host TO Network Long*/
	serv_addr.sin_port = htons(SERV_UDP_PORT);  /*Host TO Network Short*/

	/*
	* Na linha seguinte vamos então efetivamente associar o socket ao porto pretendido.
	* Para isso usamos a função bind() e é onde será feita a verificação se o porto
	* já está ou não a ser utilizado. Como sabem, só podemos ter uma aplicação a usar
	* o mesmo porto, logo, se o porto já estiver a ser usado então a aplicação será
	* terminada e apresentamos a mensagem de erro, neste caso: "Impossibilidade de
	* registar-se para escuta"
	*/
	if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR)
		Abort("Impossibilidade de registar-se para escuta");


	/*================ PASSA A ATENDER CLIENTES INTERACTIVAMENTE =============*/
	
	/*
	* Nas linhas seguintes vamos atender os clientes.
	* Estamos a usar um ciclo "infinito" (ciclo while) pois o objetivo do servidor
	* é estar sempre a atender clientes.
	* 
	* NOTA: não tem necessariamente que ser sempre assim. Podemos querer ter um servidor
	*		que só atende X numero de clientes.
	*/

	while (1) {

		fprintf(stderr, "<SER1>Esperando datagrama...\n");

		/*
		* Nas linhas seguintes vamos receber a mensagem do cliente e apresentá-la na consola.
		*
		* NOTA: notem que os últimos parametros da função recvfrom() já não estão a NULL.
		*		Isto porque, para a resolução do exercício 2, precisamos de saber de onde veio
		*		a mensagem para a podermos enviar de volta.
		* 
		*		Para sabermos de onde a mensagem veio vamos ter ajuda da função recvfrom().
		*		Por isso, criamos uma variável do tipo struct sockaddr_in e metemo-la nos
		*		dois últimos parametros da função recevfrom(). Será a própria função recevfrom()
		*		que irá preencher essa estrutura com os dados (IP e porto) de onde o datagrama
		*		foi recebido, ou seja, passamos uma estrutura vazia e a função recevfrom() trata
		*		de a preencher com os dados que queremos!
		* 
		*		Pensem nisto como um envelope:
		*		1º criamos um envelope vazio
		*		2º colocamos o envelope vazio na função recevfrom()
		*		3º a função recevfrom() preenche o envelope com os dados que queremos
		*		4º depois só temos que usar o envelope pois os dados (IP e porto) já lá estão
		*
		* NOTA2: MUITA ATENÇÃO para a linha 143!!!
		*		 Nesta linha estamos a terminar a cadeia de caracteres para podermos apresentar
		*		 corretamente no printf. A explicação está no pdf e irei explicar melhor nas
		*		 minhas aulas práticas (é mais facil explicar dando um exemplo prático)
		*/
		length_addr = sizeof(cli_addr);
		nbytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&cli_addr, &length_addr);

		if (nbytes == SOCKET_ERROR)
			Abort("Erro na recepcao de datagrams");

		buffer[nbytes] = '\0'; /*Termina a cadeia de caracteres recebidos com '\0'*/

		printf("\n<SER1>Mensagem recebida {%s}\n", buffer);

		/*====================== ENVIA MENSAGEM AO CLIENTE ==================*/

		/*
		* Na linha seguinte vamos usar a função sendto() para enviar a resposta ao cliente.
		* Lembrem-se que sempre que queremos enviar um datagrama temos que especificar um
		* endereço IP e porto de destino. 
		* E onde está o IP e porto do cliente? Está na estrutura que passamos para a função
		* recevfrom e que esta preencheu por nós. Portanto, agora só temos que a usar na função
		* sendto()... fácil não? :) 
		* 
		* NOTA: percebo que isto não seja tão trivial e fácil de entender por isso estou sempre
		*		disponível para vos ajudar!
		*/
		if (sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&cli_addr, sizeof(cli_addr)) == SOCKET_ERROR)
			Abort("O subsistema de comunicacao nao conseguiu aceitar o datagrama");

		printf("<SER1>Mensagem enviada ... a entrega nao e' confirmada.\n");

	}

}

/*________________________________ Abort________________________________________
  Mostra uma mensagem de erro e o código associado ao ultimo erro com Winsocks.
  Termina a aplicacao com "exit status" a 1 (constante EXIT_FAILURE)
________________________________________________________________________________*/

void Abort(char* msg)
{
	fprintf(stderr, "<SERV>Erro fatal: <%s> (%d)\n", msg, WSAGetLastError());
	exit(EXIT_FAILURE);
}