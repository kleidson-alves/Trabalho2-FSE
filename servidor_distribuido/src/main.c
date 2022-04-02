#include <stdio.h>
#include <unistd.h>
#include <wiringPi.h>
#include <ncurses.h>
#include <pthread.h>
#include <string.h>
#include "dht22.h"

#include "cJSON_interface.h"
#include "cliente_tcp.h"
#include "servidor_tcp.h"

JSONData info;
StateSensor estados_anteriores;

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

void enviaJson(StateSensor estados) {
    cJSON* estados_json = buildJson(estados, info.porta_servidor_distribuido);
    char* mensagem = cJSON_Print(estados_json);
    envia(info.ip_servidor_central, info.porta_servidor_central, mensagem);
}

int comparaEstados(StateSensor estado1, StateSensor estado2) {
    if (estado1.estado_entrada != estado2.estado_entrada || estado1.estado_saida != estado2.estado_saida)
        return 1;

    if (estado1.fumaca != estado2.fumaca || estado1.porta != estado2.porta || estado1.presenca != estado2.presenca)
        return 1;

    if (estado1.janela01 != estado2.janela01 || estado1.janela02 != estado2.janela02)
        return 1;

    return 0;
}

StateSensor carrega_estados() {

    StateSensor estados;
    int sensor_entrada = encontra_gpio(info.inputs, info.qntd_inputs, "contagem", 1);
    int sensor_saida = encontra_gpio(info.inputs, info.qntd_inputs, "contagem", 2);
    int sensor_presenca = encontra_gpio(info.inputs, info.qntd_inputs, "presenca", 1);
    int sensor_fumaca = encontra_gpio(info.inputs, info.qntd_inputs, "fumaca", 1);
    int sensor_janela1 = encontra_gpio(info.inputs, info.qntd_inputs, "janela", 1);
    int sensor_janela2 = encontra_gpio(info.inputs, info.qntd_inputs, "janela", 2);
    int sensor_ar_cond = encontra_gpio(info.outputs, info.qntd_outputs, "ar-condicionado", 1);
    int sensor_lampada1 = encontra_gpio(info.outputs, info.qntd_outputs, "lampada", 1);
    int sensor_lampada2 = encontra_gpio(info.outputs, info.qntd_outputs, "lampada", 2);
    int sensor_lampada_corredor = encontra_gpio(info.outputs, info.qntd_outputs, "lampada", 3);
    int sensor_porta = encontra_gpio(info.inputs, info.qntd_inputs, "porta", 1);
    int sensor_aspersor = encontra_gpio(info.outputs, info.qntd_outputs, "aspersor", 1);

    estados.estado_entrada = read_sensor_value(sensor_entrada);
    estados.estado_saida = read_sensor_value(sensor_saida);
    estados.presenca = read_sensor_value(sensor_presenca);
    estados.fumaca = read_sensor_value(sensor_fumaca);
    estados.janela01 = read_sensor_value(sensor_janela1);
    estados.janela02 = read_sensor_value(sensor_janela2);
    estados.ar_cond = read_sensor_value(sensor_ar_cond);
    estados.lampada1 = read_sensor_value(sensor_lampada1);
    estados.lampada2 = read_sensor_value(sensor_lampada2);
    estados.lampada_corredor = read_sensor_value(sensor_lampada_corredor);

    if (sensor_porta != -1) {
        estados.porta = read_sensor_value(sensor_porta);
        estados.aspersor = read_sensor_value(sensor_aspersor);
    }
    else {
        estados.porta = 0;
        estados.aspersor = 0;
    }

    return estados;
}

void envia_estados_iniciais() {

    memset(&estados_anteriores, 0, sizeof(estados_anteriores));
    cJSON* json = buildJson(estados_anteriores, info.porta_servidor_distribuido);
    char* mensagem = cJSON_Print(json);

    char* nome_andar = cJSON_Print(buildJsonToName(info.nome));
    while (envia(info.ip_servidor_central, info.porta_servidor_central, mensagem) == -1) {
        printf("Aguardando conectar-se ao servidor_central\n");
        sleep(2);
    }

    envia(info.ip_servidor_central, info.porta_servidor_central, nome_andar);
}


void* observa_sensores(void* args) {
    int cont = 0;
    StateSensor estados = *(StateSensor*)args;
    estados = estados_anteriores;

    while (1) {

        if (cont % 9 == 0) {
            cont = 0;
            estados = carrega_estados();
        }
        else {
            int sensor_entrada = encontra_gpio(info.inputs, info.qntd_inputs, "contagem", 1);
            int sensor_saida = encontra_gpio(info.inputs, info.qntd_inputs, "contagem", 2);

            estados.estado_entrada = read_sensor_value(sensor_entrada);
            estados.estado_saida = read_sensor_value(sensor_saida);
        }
        if (comparaEstados(estados, estados_anteriores))
            enviaJson(estados);

        estados_anteriores = estados;
        cont++;
        usleep(50000);
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
    StateSensor args;
    cJSON* json;
    JSONMessage solicitacao;

    envia_estados_iniciais();

    pthread_create(&(thread), NULL, &observa_sensores, &args);

    inicializaEscuta(info.porta_servidor_distribuido);

    while (1) {
        json = obterMensagem();
        solicitacao = parseMessage(json);
        int gpio = encontra_gpio(info.outputs, info.qntd_outputs, solicitacao.sensor, solicitacao.numero);
        write_sensor_value(gpio, solicitacao.comand);
    }

    finalizaEscuta();

    return 0;
}
