/*=========================== Cliente basico TCP ===============================
Este cliente destina-se a enviar mensagens passadas na linha de comando, sob
a forma de um argumento, para um servidor especifico cujo socket e' fornecido através
da linha de comando. Tambem e' aguarda confirmacao (trata-se do comprimento da mensagem).

O protocolo usado e' o TCP.
==============================================================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock.h>

#pragma comment(lib, "Ws2_32.lib")

#define BUFFERSIZE     4096

void Abort(char* msg, SOCKET s);

int main(int argc, char* argv[]) {

	SOCKET sock = INVALID_SOCKET;
	int msg_len, nbytes, iResult;
	struct sockaddr_in serv_addr;
	char buffer[BUFFERSIZE];
	WSADATA wsaData;

	/*========================= TESTA A SINTAXE =========================*/

	/*
	* Verifica se o número de argumentos passados via linha de comandos é o 
	* necessário para a correta utilização da aplicação.
	*/
	if (argc != 4) {
		fprintf(stderr, "<CLI> Sintaxe: %s \"frase_a_enviar\" ip_destino porto_destino\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/*=============== INICIA OS WINSOCKS ==============*/

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		getchar();
		exit(1);
	}


	/*=============== ABRE SOCKET PARA CONTACTAR O SERVIDOR ==============*/

	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
		Abort("Impossibilidade de abrir socket", sock);


	/*================= PREENCHE ENDERECO DO SERVIDOR ====================*/

	/*
	* Tenham atenção às funções auxiliares que permitem "traduzir" strings
	* para algo (IP e Porto) que sejam reconhecidos pelas funções usadas na
	* comunicação via winsocks.
	* 
	* NOTA: muita gente falhou isto no teste de UDP.
	*/
	memset((char*)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[2]);
	serv_addr.sin_port = htons(atoi(argv[3]));


	/*========================== ESTABELECE LIGACAO ======================*/
	
	/*
	* Visto que o protocolo TCP é orientado a ligação (ao contrário do UDP),
	* temos que primeiro estabelecer uma conexão com o endereço que queremos
	* comunicar e só depois podemos enviar/receber dados.
	* 
	* Para isso é usada a função connect.
	*/
	if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR)
		Abort("Impossibilidade de estabelecer ligacao", sock);

	
	/*====================== ENVIA MENSAGEM AO SERVIDOR ==================*/

	/*
	* Nas linhas seguintes vamos enviar dados (NOTEM que agora não falamos em
	* pacotes, uma vez que TCP é orientado a fluxo de bytes).
	* 
	* Depois de enviar (usando a função send()) estamos a validar se o número
	* de bytes enviados coincide com o número de bytes que pretendiamos enviar
	* inicialmente. Senão então mostramos uma mensagem de erro apropriada.
	* 
	* Exemplo:
	* Queremos enviar o seguinte conjunto de bytes: Ola
	* SE (nº de bytes enviados == SOCKET_ERROR)
	*	ENTÃO
	*		ocorreu um erro e mostramos mensagem de erro
	*	SENAO
	*		SE (nº de bytes enviados < nº de bytes que queriamos enviar) 
	*			ENTÃO
	*				os dados foram truncados e mostramos mensagem de erro
	*			SENÃO
	*				quer dizer que o nº de bytes enviados = nº de bytes que
	*				queriamos enviar e por isso está tudo OK
	*/

	msg_len = strlen(argv[1]);
	nbytes = send(sock, argv[1], msg_len, 0);

	if (nbytes == SOCKET_ERROR)
		Abort("Impossibilidade de transmitir mensagem...", sock);
	else if (nbytes < msg_len)
		fprintf(stderr, "<CLI> Mensagem truncada...\n");
	else
		fprintf(stderr, "<CLI> Mensagem \"%s\" enviada\n", argv[1]);


	/*========================== ESPERA CONFIRMACAO =======================*/

	nbytes = recv(sock, buffer, sizeof(buffer), 0);

	if (nbytes == SOCKET_ERROR)
		Abort("Impossibilidade de receber confirmacao", sock);

	buffer[nbytes] = '\0';

	printf("<CLI> Confirmacao recebida {%s}.\n", buffer);

	/*=========================== FECHA SOCKET ============================*/
	
	closesocket(sock);

	exit(EXIT_SUCCESS);
}


/*________________________________ Abort________________________________________
Mostra a mensagem de erro associada ao ultimo erro dos Winsock e abandona com
"exit status" a 1
_______________________________________________________________________________
*/
void Abort(char* msg, SOCKET s)
{
	fprintf(stderr, "<CLI> Erro fatal: <%d>\n", WSAGetLastError());

	if (s != INVALID_SOCKET)
		closesocket(s);

	exit(EXIT_FAILURE);
}

