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
    int porta_servidor_central;
    int qntd_inputs;
    int qntd_outputs;
    struct IO* outputs;
    struct IO* inputs;
    struct IO* sensores;
} JSONData;


void printIOData(IO g);
void buildStructList(IO* result, cJSON* list, char type);
int parse(char* filename);
JSONData getJSONData();

#endif