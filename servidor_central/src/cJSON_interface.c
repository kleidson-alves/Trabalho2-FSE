#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "cJSON_interface.h"

JSONData parseJson(cJSON* cjson) {
    JSONData json_data;
    memset(&json_data, 0, sizeof(json_data));

    json_data.estado_entrada = cJSON_GetObjectItem(cjson, "entrada")->valueint;
    json_data.estado_saida = cJSON_GetObjectItem(cjson, "saida")->valueint;
    json_data.fumaca = cJSON_GetObjectItem(cjson, "fumaca")->valueint;
    json_data.janela01 = cJSON_GetObjectItem(cjson, "janela1")->valueint;
    json_data.janela02 = cJSON_GetObjectItem(cjson, "janela2")->valueint;
    json_data.porta = cJSON_GetObjectItem(cjson, "porta")->valueint;
    json_data.presenca = cJSON_GetObjectItem(cjson, "presenca")->valueint;
    json_data.distribuido_porta = cJSON_GetObjectItem(cjson, "porta_servidor_distribuido")->valueint;
    json_data.lampada1 = cJSON_GetObjectItem(cjson, "lampada1")->valueint;
    json_data.lampada2 = cJSON_GetObjectItem(cjson, "lampada2")->valueint;
    json_data.lampada_corredor = cJSON_GetObjectItem(cjson, "lampada_corredor")->valueint;
    json_data.ar_cond = cJSON_GetObjectItem(cjson, "ar-condicionado")->valueint;
    json_data.aspersor = cJSON_GetObjectItem(cjson, "aspersor")->valueint;

    return json_data;
}

char* buildMessage(char* name, int num, int comand) {
    cJSON* json = cJSON_CreateObject();
    cJSON* sensor = NULL;
    cJSON* num_json = NULL;
    cJSON* comand_json = NULL;

    sensor = cJSON_CreateString(name);
    num_json = cJSON_CreateNumber(num);
    comand_json = cJSON_CreateNumber(comand);

    cJSON_AddItemToObject(json, "sensor", sensor);
    cJSON_AddItemToObject(json, "numero", num_json);
    cJSON_AddItemToObject(json, "comando", comand_json);

    char* message = cJSON_Print(json);
    return message;
}

JSONMessage parseMessage(char* message) {
    cJSON* json;
    JSONMessage data;
    json = cJSON_Parse(message);
    data.sensor = cJSON_GetObjectItem(json, "sensor")->valuestring;
    data.numero = cJSON_GetObjectItem(json, "numero")->valueint;
    data.comand = cJSON_GetObjectItem(json, "comando")->valueint;

    return data;
}



