#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>
#include <pthread.h>
#include <string.h>
#include "dht22.h"

#include "cJSON_interface.h"
#include "cliente_tcp.h"
#include "servidor_tcp.h"

typedef struct thread_args {
    int estado_entrada;
    int estado_saida;
    int presenca;
    int fumaca;
    int janela01;
    int janela02;
    int porta;
}thread_args;


JSONData info;

int encontra_gpio(IO* listaIO, int tamanho_lista, char* nome_sensor, int numero_sensor) {
    int num = 1;
    for (int i = 0; i < tamanho_lista; i++) {
        if (strcmp(nome_sensor, listaIO[i].type) == 0) {
            if (num == numero_sensor)
                return listaIO[i].gpio;
            num++;
        }
    }
    return -1;
}

void enviaJson(thread_args estados) {
    cJSON* estados_json = cJSON_CreateObject();
    cJSON* entrada = NULL;
    cJSON* saida = NULL;
    cJSON* presenca = NULL;
    cJSON* fumaca = NULL;
    cJSON* janela1 = NULL;
    cJSON* janela2 = NULL;
    cJSON* porta = NULL;
    cJSON* distribuido_porta = NULL;

    entrada = cJSON_CreateNumber(estados.estado_entrada);
    saida = cJSON_CreateNumber(estados.estado_saida);
    presenca = cJSON_CreateNumber(estados.presenca);
    fumaca = cJSON_CreateNumber(estados.fumaca);
    janela1 = cJSON_CreateNumber(estados.janela01);
    janela2 = cJSON_CreateNumber(estados.janela02);
    porta = cJSON_CreateNumber(estados.porta);
    distribuido_porta = cJSON_CreateNumber(info.porta_servidor_distribuido);


    cJSON_AddItemToObject(estados_json, "entrada", entrada);
    cJSON_AddItemToObject(estados_json, "saida", saida);
    cJSON_AddItemToObject(estados_json, "presenca", presenca);
    cJSON_AddItemToObject(estados_json, "fumaca", fumaca);
    cJSON_AddItemToObject(estados_json, "janela1", janela1);
    cJSON_AddItemToObject(estados_json, "janela2", janela2);
    cJSON_AddItemToObject(estados_json, "porta", porta);
    cJSON_AddItemToObject(estados_json, "porta_servidor_distribuido", distribuido_porta);

    char* mensagem = cJSON_Print(estados_json);
    envia(info.ip_servidor_central, info.porta_servidor_central, mensagem);
}

int comparaEstados(thread_args estado1, thread_args estado2) {
    if (estado1.estado_entrada != estado2.estado_entrada || estado1.estado_saida != estado2.estado_saida)
        return 1;

    if (estado1.fumaca != estado2.fumaca || estado1.porta != estado2.porta || estado1.presenca != estado2.presenca)
        return 1;

    if (estado1.janela01 != estado2.janela01 || estado1.janela02 != estado2.janela02)
        return 1;

    return 0;
}


void* observa_sensores(void* args) {
    int cont = 0;
    thread_args estados = *(thread_args*)args;
    thread_args estados_anteriores;

    estados_anteriores.estado_entrada = 0;
    estados_anteriores.fumaca = 0;
    estados_anteriores.janela01 = 0;
    estados_anteriores.janela02 = 0;
    estados_anteriores.porta = 0;
    estados_anteriores.presenca = 0;

    estados = estados_anteriores;

    while (1) {

        int sensor_entrada = encontra_gpio(info.inputs, info.qntd_inputs, "contagem", 1);
        int sensor_saida = encontra_gpio(info.inputs, info.qntd_inputs, "contagem", 2);

        estados.estado_entrada = read_sensor_value(sensor_entrada);
        estados.estado_saida = read_sensor_value(sensor_saida);

        if (cont % 10 == 0) {
            cont = 0;
            estados.presenca = read_sensor_value(encontra_gpio(info.inputs, info.qntd_inputs, "presenca", 1));
            estados.fumaca = read_sensor_value(encontra_gpio(info.inputs, info.qntd_inputs, "fumaca", 1));
            estados.janela01 = read_sensor_value(encontra_gpio(info.inputs, info.qntd_inputs, "janela", 1));
            estados.janela02 = read_sensor_value(encontra_gpio(info.inputs, info.qntd_inputs, "janela", 2));

            int sensor_porta = encontra_gpio(info.inputs, info.qntd_inputs, "porta", 1);

            if (sensor_porta != -1) {
                estados.porta = read_sensor_value(sensor_porta);
            }
        }
        if (comparaEstados(estados, estados_anteriores))
            enviaJson(estados);

        estados_anteriores = estados;
        cont++;
        usleep(10000);

    }
}

int main(int argc, char** argv) {

    if (argc != 2) {
        printf("make run FILE=<arquivo_entrada>\n");
        return 0;
    }

    if (parse(argv[1]) == -1)
        return 0;

    info = getJSONData();

    initWiringPi();
    pthread_t thread;
    thread_args args;

    pthread_create(&(thread), NULL, &observa_sensores, &args);

    cJSON* json;
    JSONMessage solicitacao;

    inicializaEscuta(info.porta_servidor_distribuido);

    while (1) {
        json = obterMensagem();
        solicitacao = parseMessage(json);
        int gpio = encontra_gpio(info.outputs, info.qntd_outputs, solicitacao.sensor, solicitacao.numero);
        write_sensor_value(gpio, solicitacao.comand);
        sleep(1);
    }

    finalizaEscuta();

    return 0;
}
