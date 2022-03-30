#ifndef SERVIDOR_TCP_H
#define SERVIDOR_TCP_H

#include "cJSON.h"


void TrataClienteTCP(int socketCliente);
cJSON* escuta(unsigned short porta);
#endif