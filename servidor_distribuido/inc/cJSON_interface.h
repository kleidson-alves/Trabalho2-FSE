#ifndef CJSON_INTERFACE_H
#define CJSON_INTERFACE_H

#include "cJSON.h"

typedef struct IO {
    char* type;
    int gpio;
    char* tag;
}IO;

typedef struct JSONData {
    char* ip_servidor_central;
    char* ip_servidor_distribuido;
    unsigned short porta_servidor_central;
    unsigned short porta_servidor_distribuido;
    int qntd_inputs;
    int qntd_outputs;
    struct IO* outputs;
    struct IO* inputs;
    struct IO* sensores;
} JSONData;

typedef struct JSONMessage {
    char* sensor;
    int numero;
    int comand;
}JSONMessage;

typedef struct StateSensor {
    int estado_entrada;
    int estado_saida;
    int presenca;
    int fumaca;
    int janela01;
    int janela02;
    int lampada1;
    int lampada2;
    int lampada_corredor;
    int porta;
    int ar_cond;
    int aspersor;
}StateSensor;

void printIOData(IO g);
void buildStructList(IO* result, cJSON* list, char type);
cJSON* buildJson(StateSensor estados, unsigned short porta_servidor_distribuido);
int parse(char* filename);
JSONMessage parseMessage(cJSON* json);
JSONData getJSONData();

#endif
