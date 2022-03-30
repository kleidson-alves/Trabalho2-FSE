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

void buildStructList(IO* result, cJSON* list, char type) {

    int size_list = cJSON_GetArraySize(list);

    for (int i = 0; i < size_list; i++) {
        cJSON* item = cJSON_GetArrayItem(list, i);

        result[i].gpio = cJSON_GetObjectItem(item, "gpio")->valueint;
        result[i].tag = cJSON_GetObjectItem(item, "tag")->valuestring;
        if (type == 't')
            result[i].type = cJSON_GetObjectItem(item, "type")->valuestring;
        else
            result[i].type = cJSON_GetObjectItem(item, "model")->valuestring;
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

    json_data.ip_servidor_central = cJSON_GetObjectItem(json, "ip_servidor_central")->valuestring;
    json_data.porta_servidor_central = cJSON_GetObjectItem(json, "porta_servidor_central")->valueint;
    json_data.ip_servidor_distribuido = cJSON_GetObjectItem(json, "ip_servidor_distribuido")->valuestring;
    json_data.porta_servidor_distribuido = cJSON_GetObjectItem(json, "porta_servidor_distribuido")->valueint;

    cJSON* lista_outputs = cJSON_GetObjectItem(json, "outputs");
    cJSON* lista_inputs = cJSON_GetObjectItem(json, "inputs");
    cJSON* lista_sensores = cJSON_GetObjectItem(json, "sensor_temepratura");

    json_data.qntd_outputs = cJSON_GetArraySize(lista_outputs);
    json_data.qntd_inputs = cJSON_GetArraySize(lista_inputs);
    int tam_lista_sensores = cJSON_GetArraySize(lista_sensores);


    json_data.outputs = (IO*)malloc(json_data.qntd_outputs * sizeof(IO));
    json_data.inputs = (IO*)malloc(json_data.qntd_inputs * sizeof(IO));
    json_data.sensores = (IO*)malloc(tam_lista_sensores * sizeof(IO));

    buildStructList(json_data.outputs, lista_outputs, 't');
    buildStructList(json_data.inputs, lista_inputs, 't');
    buildStructList(json_data.sensores, lista_sensores, 'm');

    return 0;
}

JSONData getJSONData() {
    return json_data;
}



