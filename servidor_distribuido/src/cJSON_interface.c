#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "cJSON_interface.h"

JSONData json_data;
cJSON* json;

void printIOData(IO g) {
    printf("type: %s || tag: %s || gpio: %d\n", g.type, g.tag, g.gpio);
}

void buildStructList(IO* result, cJSON* list) {

    int size_list = cJSON_GetArraySize(list);

    for (int i = 0; i < size_list; i++) {
        cJSON* item = cJSON_GetArrayItem(list, i);

        result[i].gpio = cJSON_GetObjectItem(item, "gpio")->valueint;
        result[i].tag = cJSON_GetObjectItem(item, "tag")->valuestring;

        result[i].type = cJSON_GetObjectItem(item, "type")->valuestring;

    }
}

int parse(char* filename) {
    FILE* file = fopen(filename, "r");
    char text[50000];
    char line[100];

    if (!file) {
        printf("Nao foi possivel abrir o arquivo\n");
        return -1;
    }

    while (fgets(line, 100, file) != NULL) {
        strcat(text, line);
    }
    fclose(file);

    json = cJSON_Parse(text);

    if (!json) {
        printf("Error before: [%s]\n", cJSON_GetErrorPtr());
        return -1;
    }

    json_data.nome = cJSON_GetObjectItem(json, "nome")->valuestring;
    json_data.ip_servidor_central = cJSON_GetObjectItem(json, "ip_servidor_central")->valuestring;
    json_data.porta_servidor_central = cJSON_GetObjectItem(json, "porta_servidor_central")->valueint;
    json_data.ip_servidor_distribuido = cJSON_GetObjectItem(json, "ip_servidor_distribuido")->valuestring;
    json_data.porta_servidor_distribuido = cJSON_GetObjectItem(json, "porta_servidor_distribuido")->valueint;

    cJSON* lista_outputs = cJSON_GetObjectItem(json, "outputs");
    cJSON* lista_inputs = cJSON_GetObjectItem(json, "inputs");
    cJSON* lista_sensores = cJSON_GetObjectItem(json, "sensor_temperatura");

    json_data.qntd_outputs = cJSON_GetArraySize(lista_outputs);
    json_data.qntd_inputs = cJSON_GetArraySize(lista_inputs);
    int tam_lista_sensores = cJSON_GetArraySize(lista_sensores);


    json_data.outputs = (IO*)malloc(json_data.qntd_outputs * sizeof(IO));
    json_data.inputs = (IO*)malloc(json_data.qntd_inputs * sizeof(IO));
    json_data.sensores = (IO*)malloc(tam_lista_sensores * sizeof(IO));

    buildStructList(json_data.outputs, lista_outputs);
    buildStructList(json_data.inputs, lista_inputs);
    buildStructList(json_data.sensores, lista_sensores);

    return 0;
}

JSONMessage parseMessage(cJSON* json) {
    JSONMessage data;
    data.sensor = cJSON_GetObjectItem(json, "sensor")->valuestring;
    data.numero = cJSON_GetObjectItem(json, "numero")->valueint;
    data.comand = cJSON_GetObjectItem(json, "comando")->valueint;

    return data;
}

cJSON* buildJson(StateSensor estados, unsigned short porta_servidor_distribuido) {
    cJSON* estados_json = cJSON_CreateObject();
    cJSON* entrada = NULL;
    cJSON* saida = NULL;
    cJSON* presenca = NULL;
    cJSON* fumaca = NULL;
    cJSON* janela1 = NULL;
    cJSON* janela2 = NULL;
    cJSON* porta = NULL;
    cJSON* lampada1 = NULL;
    cJSON* lampada2 = NULL;
    cJSON* lampada_corredor = NULL;
    cJSON* ar_cond = NULL;
    cJSON* aspersor = NULL;
    cJSON* distribuido_porta = NULL;
    cJSON* temperatura = NULL;
    cJSON* umidade = NULL;

    entrada = cJSON_CreateNumber(estados.estado_entrada);
    saida = cJSON_CreateNumber(estados.estado_saida);
    presenca = cJSON_CreateNumber(estados.presenca);
    fumaca = cJSON_CreateNumber(estados.fumaca);
    janela1 = cJSON_CreateNumber(estados.janela01);
    janela2 = cJSON_CreateNumber(estados.janela02);
    porta = cJSON_CreateNumber(estados.porta);
    lampada1 = cJSON_CreateNumber(estados.lampada1);
    lampada2 = cJSON_CreateNumber(estados.lampada2);
    lampada_corredor = cJSON_CreateNumber(estados.lampada_corredor);
    ar_cond = cJSON_CreateNumber(estados.ar_cond);
    aspersor = cJSON_CreateNumber(estados.aspersor);
    temperatura = cJSON_CreateNumber(estados.temp);
    umidade = cJSON_CreateNumber(estados.umidade);


    distribuido_porta = cJSON_CreateNumber(porta_servidor_distribuido);


    cJSON_AddItemToObject(estados_json, "entrada", entrada);
    cJSON_AddItemToObject(estados_json, "saida", saida);
    cJSON_AddItemToObject(estados_json, "presenca", presenca);
    cJSON_AddItemToObject(estados_json, "fumaca", fumaca);
    cJSON_AddItemToObject(estados_json, "janela1", janela1);
    cJSON_AddItemToObject(estados_json, "janela2", janela2);
    cJSON_AddItemToObject(estados_json, "porta", porta);
    cJSON_AddItemToObject(estados_json, "lampada1", lampada1);
    cJSON_AddItemToObject(estados_json, "lampada2", lampada2);
    cJSON_AddItemToObject(estados_json, "lampada_corredor", lampada_corredor);
    cJSON_AddItemToObject(estados_json, "ar-condicionado", ar_cond);
    cJSON_AddItemToObject(estados_json, "aspersor", aspersor);
    cJSON_AddItemToObject(estados_json, "porta_servidor_distribuido", distribuido_porta);
    cJSON_AddItemToObject(estados_json, "temperatura", temperatura);
    cJSON_AddItemToObject(estados_json, "umidade", umidade);

    return estados_json;
}

cJSON* buildJsonToName(char* name) {
    cJSON* json_name = cJSON_CreateObject();
    cJSON* nome = NULL;

    nome = cJSON_CreateString(name);
    cJSON_AddItemToObject(json_name, "nome", nome);


    return json_name;
}



JSONData getJSONData() {
    return json_data;
}



