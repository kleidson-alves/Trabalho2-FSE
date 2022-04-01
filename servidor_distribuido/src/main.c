#include <stdio.h>
#include <unistd.h>
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

void carrega_estados() {
    int sensor_entrada = encontra_gpio(info.inputs, info.qntd_inputs, "contagem", 1);
    int sensor_saida = encontra_gpio(info.inputs, info.qntd_inputs, "contagem", 2);

    estados_anteriores.estado_entrada = read_sensor_value(sensor_entrada);
    estados_anteriores.estado_saida = read_sensor_value(sensor_saida);

    estados_anteriores.presenca = read_sensor_value(encontra_gpio(info.inputs, info.qntd_inputs, "presenca", 1));
    estados_anteriores.fumaca = read_sensor_value(encontra_gpio(info.inputs, info.qntd_inputs, "fumaca", 1));
    estados_anteriores.janela01 = read_sensor_value(encontra_gpio(info.inputs, info.qntd_inputs, "janela", 1));
    estados_anteriores.janela02 = read_sensor_value(encontra_gpio(info.inputs, info.qntd_inputs, "janela", 2));
    estados_anteriores.ar_cond = read_sensor_value(encontra_gpio(info.outputs, info.qntd_outputs, "ar-condicionado", 1));
    estados_anteriores.lampada1 = read_sensor_value(encontra_gpio(info.outputs, info.qntd_outputs, "lampada", 1));
    estados_anteriores.lampada2 = read_sensor_value(encontra_gpio(info.outputs, info.qntd_outputs, "lampada", 2));
    estados_anteriores.lampada_corredor = read_sensor_value(encontra_gpio(info.outputs, info.qntd_outputs, "lampada", 3));


    int sensor_porta = encontra_gpio(info.inputs, info.qntd_inputs, "porta", 1);
    int sensor_aspersor = encontra_gpio(info.outputs, info.qntd_outputs, "aspersor", 1);
    if (sensor_porta != -1) {
        estados_anteriores.porta = read_sensor_value(sensor_porta);
        estados_anteriores.aspersor = read_sensor_value(sensor_aspersor);
    }
    else {
        estados_anteriores.porta = -1;
    }

    char* mensagem = cJSON_Print(buildJson(estados_anteriores, info.porta_servidor_distribuido));
    while (envia(info.ip_servidor_central, info.porta_servidor_central, mensagem) == -1) {
        printf("Aguardando conectar-se ao servidor_central\n");
        sleep(2);
    }

}


void* observa_sensores(void* args) {
    int cont = 0;
    StateSensor estados = *(StateSensor*)args;
    memset(&estados, 0, sizeof(estados));

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
    StateSensor args;

    carrega_estados();

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
