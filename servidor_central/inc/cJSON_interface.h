#ifndef CJSON_INTERFACE_H
#define CJSON_INTERFACE_H

#include "cJSON.h"

typedef struct IO {
    char* type;
    int gpio;
    char* tag;
}IO;

typedef struct JSONMessage {
    char* sensor;
    int numero;
    int comand;
}JSONMessage;


typedef struct JSONData {
    int estado_entrada;
    int estado_saida;
    int presenca;
    int fumaca;
    int janela01;
    int janela02;
    int porta;
    int lampada1;
    int lampada2;
    int lampada_corredor;
    int ar_cond;
    int aspersor;
    unsigned short distribuido_porta;
} JSONData;

JSONData parseJson(cJSON* json);
char* buildMessage(char* name, int num, int comand);
JSONMessage parseMessage(char* message);

#endif