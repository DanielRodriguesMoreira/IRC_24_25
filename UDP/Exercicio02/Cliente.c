/*=========================== Cliente basico UDP ===============================
Este cliente destina-se a enviar mensagens passadas na linha de comando, sob
a forma de um argumento, para um servidor especifico cuja locacao e' dada
pelas seguintes constantes: SERV_HOST_ADDR (endereco IP) e SERV_UDP_PORT (porto)

O protocolo usado e' o UDP.
==============================================================================*/

#include <winsock.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

#define SERV_HOST_ADDR "127.0.0.1"
#define SERV_UDP_PORT  6000
#define BUFFERSIZE     4096

void Abort(char* msg);

/*________________________________ main _______________________________________
*/

int main(int argc, char* argv[])
{
	SOCKET sockfd;
	int msg_len, iResult;
	struct sockaddr_in serv_addr;
	WSADATA wsaData;
	int nbytes;
	char buffer[BUFFERSIZE];

	/*========================= TESTA A SINTAXE =========================*/

	/*
	* Apesar de isto n�o ser necess�rio para a resolu��o dos exerc�cios,
	* � algo que deve ser feito para validar se a aplica��o � executada
	* corretamente
	*/
	if (argc != 2) {
		fprintf(stderr, "Sintaxe: %s frase_a_enviar\n", argv[0]);
		getchar();
		exit(EXIT_FAILURE);
	}

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

	/*=============== CRIA SOCKET PARA ENVIO/RECEPCAO DE DATAGRAMAS ==============*/

	/*
	* Nas linhas seguintes estamos a criar o Socket que ser� usado na comunica��o
	* entre aplica��es.
	*/
	sockfd = socket(PF_INET, SOCK_DGRAM, 0);
	if (sockfd == INVALID_SOCKET)
		Abort("Impossibilidade de criar socket");

	/*================= PREENCHE ENDERECO DO SERVIDOR ====================*/

	/*
	* Nas linhas seguintes estamos a preencher o endere�o do servidor. Lembrem-se
	* que o protocolo UDP � como uma carta de correio e por isso t�m sempre que
	* especificar o endere�o IP e porto do destinat�rio, ou seja, para onde querem
	* enviar o datagrama.
	* Notem que existem fun��es "auxiliares" que precisamos de usar para especificar
	* o IP e porto. Estas fun��es permitem converter caracteres/inteiros/... para algo
	* que seja reconhecido pelos sockets (mais explica��o no pdf disponibilizado)
	* 
	* NOTA: na linha 101 estamos basicamente a limpar a estrutura serv_addr, ou seja,
	*       estamos a zerar todos os bytes dessa estrutura.
	*		Isto � importante para evitarmos ter lixo nas propriedades que ir�o
	*		guardar o endere�o IP e porto do servidor.
	* 
	* NOTA2: como isto � puramente C (n�o � algo introduzido pelos sockets), n�o ser�
	*		 alvo de avalia��o nos testes pr�ticos da minha turma. No entanto, devem
	*		 ter uma no��o do que a fun��o memset faz.
	* 
	* NOTA3: mais uma vez falo apenas por mim, n�o garanto que n�o apare�a em testes
	* de outros professores!
	*/
	memset((char*)&serv_addr, 0, sizeof(serv_addr)); /*Coloca a zero todos os bytes*/
	serv_addr.sin_family = AF_INET; /*Address Family: Internet*/
	serv_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR); /*IP no formato "dotted decimal" => 32 bits*/
	serv_addr.sin_port = htons(SERV_UDP_PORT); /*Host TO Netowork Short*/

	/*====================== ENVIA MENSAGEM AO SERVIDOR ==================*/

	/*
	* Nas linhas seguintes vamos enviar a mensagem para o servidor. Para isso temos que usar
	* a fun��o sendto(...) e temos que especificar todos os parametros.
	* 
	* NOTA: vejam a especifica��o de cada parametro no pdf que disponibilizei no Github.
	* 
	* NOTA2: tomem especial aten��o para o tipo de parametros que devem passar, se a fun��o
	*		 sendto() estiver � espera de um ponteiro ent�o t�m que passar a refer�ncia da
	*		 vari�vel (usando o caracter &).
	*		 Muita gente falha isto nos testes pr�ticos.
	* 
	* NOTA3: tomem aten��o ao sizeof() vs strlen()
	*		 Mais uma vez, olhem para o pdf pois est� l� a diferen�a entre eles.
	*		 Se mesmo assim tiverem d�vidas podem sempre entrar em contacto comigo.
	*/
	msg_len = strlen(argv[1]);

	if (sendto(sockfd, argv[1], msg_len, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR)
		Abort("O subsistema de comunicacao nao conseguiu aceitar o datagrama");

	printf("<CLI1>Mensagem enviada ... a entrega nao e' confirmada.\n");

	/*========================= RECEBER MENSAGEM ===========================*/

	/*
	* Nas linhas seguintes vamos receber a resposta do servidor e apresent�-la na consola.
	* 
	* NOTA: notem que os �ltimos parametros da fun��o recvfrom() est�o a NULL.
	*		Para a resolu��o do exerc�cio 2 est�o bem assim, no entanto, isto ter� que
	*		ser alterado para os exerc�cios seguintes.
	*		Irei explicar melhor estes parametros na fun��o recvfrom() do lado do servidor.
	*		E, mais uma vez, t�m a explica��o no pdf que disponibilizei.
	* 
	* NOTA2: MUITA ATEN��O para a linha 151!!!
	*		 Nesta linha estamos a terminar a cadeia de caracteres para podermos apresentar
	*		 corretamente no printf. A explica��o est� no pdf e irei explicar melhor nas
	*		 minhas aulas pr�ticas (� mais facil explicar dando um exemplo pr�tico)
	*/
	nbytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);

	if (nbytes == SOCKET_ERROR)
		Abort("Erro na recepcao de datagrams");

	buffer[nbytes] = '\0'; /*Termina a cadeia de caracteres recebidos com '\0'*/

	printf("\n<CLI>Mensagem recebida {%s}\n", buffer);

	/*========================= FECHA O SOCKET ===========================*/

	/*
	* Na linha seguinta vamos fechar o socket. Isto pode parecer algo insignificante mas n�o o �.
	* Temos sempre que libertar recursos que j� n�o iremos utilizar.
	*/
	closesocket(sockfd);

	printf("\n");
	exit(EXIT_SUCCESS);
}

/*________________________________ Abort________________________________________
  Mostra uma mensagem de erro e o c�digo associado ao ultimo erro com Winsocks.
  Termina a aplicacao com "exit status" a 1 (constante EXIT_FAILURE)
________________________________________________________________________________*/

void Abort(char* msg)
{

	fprintf(stderr, "<CLI1>Erro fatal: <%s> (%d)\n", msg, WSAGetLastError());
	exit(EXIT_FAILURE);

}
