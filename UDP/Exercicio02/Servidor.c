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
	* Este � o primeiro passo que tem que ser feito quando se pretende
	* criar uma aplica��o recorrendo a Windows Sockets.
	* No fundo estamos a indicar que a nossa aplica��o pretende usar
	* Windows Sockets e ser� retornado um c�digo de erro caso n�o
	* seja poss�vel.
	*
	* NOTA: esta fun��o n�o est� no formul�rio que ter�o acesso
	* no dia do teste e por isso n�o ser� alvo de avalia��o nos testes
	* pr�ticos da minha turma.
	*
	* NOTA2: falo apenas por mim, n�o garanto que n�o apare�a em testes
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
	* Nas linhas seguintes estamos a criar o Socket que ser� usado na comunica��o
	* entre aplica��es.
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
	* NOTA: notem que a linha 82, onde especificamos o IP, � diferente do que fazemos
	*		do lado do cliente!
	*		Isto porque, do lado do cliente estamos a especificar qual o IP do servidor
	*		a quem queremos enviar um datagram, enquanto que, do lado do servidor,
	*		estamos a especificar que pretendemos receber datagramas de qualquer
	*		interface de rede (da� usarmos a constante INADDR_ANY).
	* 
	* NOTA2: tal como no cliente, aqui tamb�m temos que usar fun��es "auxiliares" para
	*		 converter o IP e porto para algo reconhecido pelos sockets.
	*/
	memset((char*)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET; /*Address Family: Internet*/
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  /*Host TO Network Long*/
	serv_addr.sin_port = htons(SERV_UDP_PORT);  /*Host TO Network Short*/

	/*
	* Na linha seguinte vamos ent�o efetivamente associar o socket ao porto pretendido.
	* Para isso usamos a fun��o bind() e � onde ser� feita a verifica��o se o porto
	* j� est� ou n�o a ser utilizado. Como sabem, s� podemos ter uma aplica��o a usar
	* o mesmo porto, logo, se o porto j� estiver a ser usado ent�o a aplica��o ser�
	* terminada e apresentamos a mensagem de erro, neste caso: "Impossibilidade de
	* registar-se para escuta"
	*/
	if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR)
		Abort("Impossibilidade de registar-se para escuta");


	/*================ PASSA A ATENDER CLIENTES INTERACTIVAMENTE =============*/
	
	/*
	* Nas linhas seguintes vamos atender os clientes.
	* Estamos a usar um ciclo "infinito" (ciclo while) pois o objetivo do servidor
	* � estar sempre a atender clientes.
	* 
	* NOTA: n�o tem necessariamente que ser sempre assim. Podemos querer ter um servidor
	*		que s� atende X numero de clientes.
	*/

	while (1) {

		fprintf(stderr, "<SER1>Esperando datagrama...\n");

		/*
		* Nas linhas seguintes vamos receber a mensagem do cliente e apresent�-la na consola.
		*
		* NOTA: notem que os �ltimos parametros da fun��o recvfrom() j� n�o est�o a NULL.
		*		Isto porque, para a resolu��o do exerc�cio 2, precisamos de saber de onde veio
		*		a mensagem para a podermos enviar de volta.
		* 
		*		Para sabermos de onde a mensagem veio vamos ter ajuda da fun��o recvfrom().
		*		Por isso, criamos uma vari�vel do tipo struct sockaddr_in e metemo-la nos
		*		dois �ltimos parametros da fun��o recevfrom(). Ser� a pr�pria fun��o recevfrom()
		*		que ir� preencher essa estrutura com os dados (IP e porto) de onde o datagrama
		*		foi recebido, ou seja, passamos uma estrutura vazia e a fun��o recevfrom() trata
		*		de a preencher com os dados que queremos!
		* 
		*		Pensem nisto como um envelope:
		*		1� criamos um envelope vazio
		*		2� colocamos o envelope vazio na fun��o recevfrom()
		*		3� a fun��o recevfrom() preenche o envelope com os dados que queremos
		*		4� depois s� temos que usar o envelope pois os dados (IP e porto) j� l� est�o
		*
		* NOTA2: MUITA ATEN��O para a linha 143!!!
		*		 Nesta linha estamos a terminar a cadeia de caracteres para podermos apresentar
		*		 corretamente no printf. A explica��o est� no pdf e irei explicar melhor nas
		*		 minhas aulas pr�ticas (� mais facil explicar dando um exemplo pr�tico)
		*/
		length_addr = sizeof(cli_addr);
		nbytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&cli_addr, &length_addr);

		if (nbytes == SOCKET_ERROR)
			Abort("Erro na recepcao de datagrams");

		buffer[nbytes] = '\0'; /*Termina a cadeia de caracteres recebidos com '\0'*/

		printf("\n<SER1>Mensagem recebida {%s}\n", buffer);

		/*====================== ENVIA MENSAGEM AO CLIENTE ==================*/

		/*
		* Na linha seguinte vamos usar a fun��o sendto() para enviar a resposta ao cliente.
		* Lembrem-se que sempre que queremos enviar um datagrama temos que especificar um
		* endere�o IP e porto de destino. 
		* E onde est� o IP e porto do cliente? Est� na estrutura que passamos para a fun��o
		* recevfrom e que esta preencheu por n�s. Portanto, agora s� temos que a usar na fun��o
		* sendto()... f�cil n�o? :) 
		* 
		* NOTA: percebo que isto n�o seja t�o trivial e f�cil de entender por isso estou sempre
		*		dispon�vel para vos ajudar!
		*/
		if (sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&cli_addr, sizeof(cli_addr)) == SOCKET_ERROR)
			Abort("O subsistema de comunicacao nao conseguiu aceitar o datagrama");

		printf("<SER1>Mensagem enviada ... a entrega nao e' confirmada.\n");

	}

}

/*________________________________ Abort________________________________________
  Mostra uma mensagem de erro e o c�digo associado ao ultimo erro com Winsocks.
  Termina a aplicacao com "exit status" a 1 (constante EXIT_FAILURE)
________________________________________________________________________________*/

void Abort(char* msg)
{
	fprintf(stderr, "<SERV>Erro fatal: <%s> (%d)\n", msg, WSAGetLastError());
	exit(EXIT_FAILURE);
}