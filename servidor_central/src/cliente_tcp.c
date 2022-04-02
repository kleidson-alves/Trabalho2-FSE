#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cliente_tcp.h"
#include "cJSON_interface.h"

char* envia(char* IP_Servidor, unsigned short servidorPorta, char* mensagem) {
    int clienteSocket;
    struct sockaddr_in servidorAddr;

    char buffer[300];
    unsigned int tamanhoMensagem;

    int bytesRecebidos;
    int totalBytesRecebidos;
    char* mensagemRetorno;

    // Criar Socket
    if ((clienteSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        printf("Erro no socket()\n");

    // Construir struct sockaddr_in
    memset(&servidorAddr, 0, sizeof(servidorAddr)); // Zerando a estrutura de dados
    servidorAddr.sin_family = AF_INET;
    servidorAddr.sin_addr.s_addr = inet_addr(IP_Servidor);
    servidorAddr.sin_port = htons(servidorPorta);

    // Connect
    if (connect(clienteSocket, (struct sockaddr*)&servidorAddr,
        sizeof(servidorAddr)) < 0)
        mensagemRetorno = buildMessage("erro", -1, -1);


    tamanhoMensagem = strlen(mensagem);

    if (send(clienteSocket, mensagem, tamanhoMensagem, 0) != tamanhoMensagem)
        printf("Erro no envio: numero de bytes enviados diferente do esperado\n");

    totalBytesRecebidos = 0;
    int erro = 0;
    while (totalBytesRecebidos < tamanhoMensagem) {
        if ((bytesRecebidos = recv(clienteSocket, buffer, 300 - 1, 0)) <= 0) {
            printf("Não recebeu o total de bytes enviados\n");
            mensagemRetorno = buildMessage("erro", -1, -1);
            erro = 1;
        }
        totalBytesRecebidos += bytesRecebidos;
        buffer[bytesRecebidos] = '\0';
    }
    if (erro == 0)
        mensagemRetorno = mensagem;
    close(clienteSocket);

    return mensagemRetorno;
}