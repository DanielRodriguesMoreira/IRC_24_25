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
	* Apesar de isto não ser necessário para a resolução dos exercícios,
	* é algo que deve ser feito para validar se a aplicação é executada
	* corretamente
	*/
	if (argc != 2) {
		fprintf(stderr, "Sintaxe: %s frase_a_enviar\n", argv[0]);
		getchar();
		exit(EXIT_FAILURE);
	}

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

	/*=============== CRIA SOCKET PARA ENVIO/RECEPCAO DE DATAGRAMAS ==============*/

	/*
	* Nas linhas seguintes estamos a criar o Socket que será usado na comunicação
	* entre aplicações.
	*/
	sockfd = socket(PF_INET, SOCK_DGRAM, 0);
	if (sockfd == INVALID_SOCKET)
		Abort("Impossibilidade de criar socket");

	/*================= PREENCHE ENDERECO DO SERVIDOR ====================*/

	/*
	* Nas linhas seguintes estamos a preencher o endereço do servidor. Lembrem-se
	* que o protocolo UDP é como uma carta de correio e por isso têm sempre que
	* especificar o endereço IP e porto do destinatário, ou seja, para onde querem
	* enviar o datagrama.
	* Notem que existem funções "auxiliares" que precisamos de usar para especificar
	* o IP e porto. Estas funções permitem converter caracteres/inteiros/... para algo
	* que seja reconhecido pelos sockets (mais explicação no pdf disponibilizado)
	* 
	* NOTA: na linha 101 estamos basicamente a limpar a estrutura serv_addr, ou seja,
	*       estamos a zerar todos os bytes dessa estrutura.
	*		Isto é importante para evitarmos ter lixo nas propriedades que irão
	*		guardar o endereço IP e porto do servidor.
	* 
	* NOTA2: como isto é puramente C (não é algo introduzido pelos sockets), não será
	*		 alvo de avaliação nos testes práticos da minha turma. No entanto, devem
	*		 ter uma noção do que a função memset faz.
	* 
	* NOTA3: mais uma vez falo apenas por mim, não garanto que não apareça em testes
	* de outros professores!
	*/
	memset((char*)&serv_addr, 0, sizeof(serv_addr)); /*Coloca a zero todos os bytes*/
	serv_addr.sin_family = AF_INET; /*Address Family: Internet*/
	serv_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR); /*IP no formato "dotted decimal" => 32 bits*/
	serv_addr.sin_port = htons(SERV_UDP_PORT); /*Host TO Netowork Short*/

	/*====================== ENVIA MENSAGEM AO SERVIDOR ==================*/

	/*
	* Nas linhas seguintes vamos enviar a mensagem para o servidor. Para isso temos que usar
	* a função sendto(...) e temos que especificar todos os parametros.
	* 
	* NOTA: vejam a especificação de cada parametro no pdf que disponibilizei no Github.
	* 
	* NOTA2: tomem especial atenção para o tipo de parametros que devem passar, se a função
	*		 sendto() estiver à espera de um ponteiro então têm que passar a referência da
	*		 variável (usando o caracter &).
	*		 Muita gente falha isto nos testes práticos.
	* 
	* NOTA3: tomem atenção ao sizeof() vs strlen()
	*		 Mais uma vez, olhem para o pdf pois está lá a diferença entre eles.
	*		 Se mesmo assim tiverem dúvidas podem sempre entrar em contacto comigo.
	*/
	msg_len = strlen(argv[1]);

	if (sendto(sockfd, argv[1], msg_len, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR)
		Abort("O subsistema de comunicacao nao conseguiu aceitar o datagrama");

	printf("<CLI1>Mensagem enviada ... a entrega nao e' confirmada.\n");

	/*========================= RECEBER MENSAGEM ===========================*/

	/*
	* Nas linhas seguintes vamos receber a resposta do servidor e apresentá-la na consola.
	* 
	* NOTA: notem que os últimos parametros da função recvfrom() estão a NULL.
	*		Para a resolução do exercício 2 estão bem assim, no entanto, isto terá que
	*		ser alterado para os exercícios seguintes.
	*		Irei explicar melhor estes parametros na função recvfrom() do lado do servidor.
	*		E, mais uma vez, têm a explicação no pdf que disponibilizei.
	* 
	* NOTA2: MUITA ATENÇÃO para a linha 151!!!
	*		 Nesta linha estamos a terminar a cadeia de caracteres para podermos apresentar
	*		 corretamente no printf. A explicação está no pdf e irei explicar melhor nas
	*		 minhas aulas práticas (é mais facil explicar dando um exemplo prático)
	*/
	nbytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);

	if (nbytes == SOCKET_ERROR)
		Abort("Erro na recepcao de datagrams");

	buffer[nbytes] = '\0'; /*Termina a cadeia de caracteres recebidos com '\0'*/

	printf("\n<CLI>Mensagem recebida {%s}\n", buffer);

	/*========================= FECHA O SOCKET ===========================*/

	/*
	* Na linha seguinta vamos fechar o socket. Isto pode parecer algo insignificante mas não o é.
	* Temos sempre que libertar recursos que já não iremos utilizar.
	*/
	closesocket(sockfd);

	printf("\n");
	exit(EXIT_SUCCESS);
}

/*________________________________ Abort________________________________________
  Mostra uma mensagem de erro e o código associado ao ultimo erro com Winsocks.
  Termina a aplicacao com "exit status" a 1 (constante EXIT_FAILURE)
________________________________________________________________________________*/

void Abort(char* msg)
{

	fprintf(stderr, "<CLI1>Erro fatal: <%s> (%d)\n", msg, WSAGetLastError());
	exit(EXIT_FAILURE);

}
