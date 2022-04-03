#ifndef SERVIDOR_TCP_H
#define SERVIDOR_TCP_H

#include "cJSON.h"

void TrataClienteTCP(int socketCliente);
void inicializaEscuta(unsigned short servidorPorta);
cJSON* obterMensagem();
void finalizaEscuta();
void encerraServidor();
#endif