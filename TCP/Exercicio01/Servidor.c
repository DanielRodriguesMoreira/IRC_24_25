/*======================= Servidor interactivo TCP ============================
Este servidor destina-se mostrar mensagens recebidas via TCP, no porto
definido pela constante SERV_TCP_PORT.
Trata-se de um servidor que envia confirmacao (o comprimento, em bytes, da
mensagem recebida).
===============================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock.h>

#pragma comment(lib, "Ws2_32.lib")

#define SERV_TCP_PORT  6000
#define BUFFERSIZE     4096

void Abort(char* msg, SOCKET s);
void AtendeCliente(SOCKET sock);

int main(int argc, char* argv[]) {

	SOCKET sock = INVALID_SOCKET, newSock = INVALID_SOCKET;
	int iResult;
	int cliaddr_len;
	struct sockaddr_in cli_addr, serv_addr;
	WSADATA wsaData;

	/*=============== INICIA OS WINSOCKS ==============*/

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		getchar();
		exit(1);
	}


	/*================== ABRE SOCKET PARA ESCUTA DE CLIENTES ================*/
	
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
		Abort("Impossibilidade de abrir socket", sock);

	
	/*=================== PREENCHE ENDERECO DE ESCUTA =======================*/
	
	memset((char*)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(SERV_TCP_PORT);


	/*====================== REGISTA-SE PARA ESCUTA =========================*/
	
	if (bind(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR)
		Abort("Impossibilidade de registar-se para escuta", sock);

	
	/*============ AVISA QUE ESTA PRONTO A ACEITAR PEDIDOS ==================*/
	
	/*
	* Esta é uma função nova (não existia em UDP) e é importante!
	* É esta função que vai colocar o socket num estado "de escuta" em que
	* permite aceitar conexões/ligações.
	* Recebe como parâmetros o socket que pretendemos colocar à escuta e o número
	* máximo de ligações em "lista de espera"
	* 
	* NOTA: atenção que este número não é o número máximo de ligações permitido!
	*		mas sim, o número máximo de ligações que podem ficar à espera até serem
	*		aceites.
	*/
	if (listen(sock, 5) == SOCKET_ERROR)
		Abort("Impossibilidade de escutar pedidos", sock);


	/*================ PASSA A ATENDER CLIENTES INTERACTIVAMENTE =============*/
	
	cliaddr_len = sizeof(cli_addr);
	while (1) {
		
		/*====================== ATENDE PEDIDO ========================*/

		/*
		* Esta é mais uma função nova!
		* 
		* O que nós vamos ter agora é um socket para cada cliente que queremos atender.
		* Isto porque estamos a usar TCP, logo têm que haver ligação, logo têm que haver
		* um socket dedicado a cada cliente.
		* 
		* A função accept() é uma função bloqueante que fica à espera de ligações.
		* Quando um cliente executa a função connect() do seu lado, a função accept() do
		* lado do servidor irá ser executada e retornará um socket dedicado a esse cliente.
		* 
		* NOTA: explicarei isto melhor na sala de aula com desenhos no quadro.
		*/
		newSock = accept(sock, (struct sockaddr*)&cli_addr, &cliaddr_len);
		if (newSock == SOCKET_ERROR)
			fprintf(stderr, "<SERV> Impossibilidade de aceitar cliente...\n");
		else {
			printf("<SERV> Ligacao aceite de {%s:%d}\r\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

			/*
			* Para simplificar o código (e preparar para os exercícios com threads)
			* temos uma função chamada AtendeCliente que recebe um socket como paramêtro
			* e trata de receber e enviar fluxos de dados (bytes) entre cliente e servidor.
			* 
			* NOTA: a implementação da função encontra-se nas linhas abaixo.
			*/
			AtendeCliente(newSock);

			closesocket(newSock);
		}
	}
}

/*___________________________ AtendeCliente _________________________*/

void AtendeCliente(SOCKET sock) {
	static char buffer[BUFFERSIZE];
	static unsigned int cont = 0;
	int nbytes, nBytesSent;

	/*==================== PROCESSA PEDIDO ==========================*/

	/*
	* Na linha seguinte vamos usar a função recv para colocar na variável
	* buffer os dados recebidos
	*/
	nbytes = recv(sock, buffer, BUFFERSIZE, 0);

	/*
	* Nas linhas seguintes estamos a usar uma instrução switch para processar
	* o número de bytes recebidos (nbytes)
	* 
	* NOTA: isto não é mais do que o if/elseif que usámos no cliente mas aqui
	*		estamos a usar um switch com vários cases.
	* 
	* NOTA2: apesar de vocês já terem dado isto noutras cadeiras, poderei explicar
	*		 nas aulas práticas (se alguém se mostrar com dúvidas, caso contrário
			 vou assumir que sabem o que é e como usar)
	*/
	switch (nbytes) {
	case SOCKET_ERROR:
		fprintf(stderr, "\n<SER> Erro na recepcao de dados...\n");
		return;
	case  0:
		fprintf(stderr, "\n<SER> O cliente nao enviou dados...\n");
		return;
	default:
		buffer[nbytes] = '\0';
		printf("\n<SER> Mensagem n. %d recebida {%s}\n", ++cont, buffer);

		/*============ ENVIA CONFIRMACAO =============*/

		/*
		* Nas linhas seguintes estamos a colocar o número de bytes recebidos 
		* na variável buffer e a enviar de volta para o cliente usando a
		* função send().
		* 
		* NOTA: mais uma vez, depois de fazer o send() estamos a validar se os
		*		dados foram enviados com sucesso.
		*/
		printf("<SER> Confirma recepcao de mensagem.\n");
		sprintf_s(buffer, BUFFERSIZE, "%d", nbytes);
		nbytes = strlen(buffer);

		nBytesSent = send(sock, buffer, nbytes, 0);
		if (nBytesSent == SOCKET_ERROR)
			fprintf(stderr, "<SER> Impossibilidade de Confirmar.\n");
		else if (nBytesSent < nbytes)
			fprintf(stderr, "<SER> Mensagem confirmada, mas truncada.\n");
		else
			printf("<SER> Mensagem confirmada.\n");
	}

}

/*________________________________ Abort________________________________________
Mostra a mensagem de erro associada ao ultimo erro no SO e abando com
"exit status" a 1
_______________________________________________________________________________
*/
void Abort(char* msg, SOCKET s)
{
	fprintf(stderr, "\a<SER >Erro fatal: <%d>\n", WSAGetLastError());

	if (s != INVALID_SOCKET)
		closesocket(s);

	exit(EXIT_FAILURE);
}
