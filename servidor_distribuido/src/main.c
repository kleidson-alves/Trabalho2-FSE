#include <stdio.h>
#include <unistd.h>
#include <wiringPi.h>
#include <ncurses.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include "dht22.h"

#include "cJSON_interface.h"
#include "cliente_tcp.h"
#include "servidor_tcp.h"

float temp, umidade, temp_ant, umidade_ant;
int altera;
JSONData info;
pthread_t thread_cliente, thread_dht22;


void trata_sinal(int signum) {
    encerraServidor();
    pthread_cancel(thread_cliente);
    pthread_cancel(thread_dht22);
    exit(0);
}


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

    int envio = envia(info.ip_servidor_central, info.porta_servidor_central, mensagem);
    if (envio == -1) {
        printf("Perdeu a conexão com o servidor central\n");
        trata_sinal(envio);
    }
}

int comparaEstados(StateSensor estado1, StateSensor estado2) {
    if (estado1.estado_entrada != estado2.estado_entrada || estado1.estado_saida != estado2.estado_saida)
        return 1;

    if (estado1.fumaca != estado2.fumaca || estado1.presenca != estado2.presenca || estado1.porta != estado2.porta)
        return 1;

    if (estado1.janela01 != estado2.janela01 || estado1.janela02 != estado2.janela02)
        return 1;

    if (estado1.lampada1 != estado2.lampada1 || estado1.lampada2 != estado2.lampada2 || estado1.lampada_corredor != estado2.lampada_corredor)
        return 1;

    if (estado1.aspersor != estado2.aspersor || estado1.ar_cond != estado2.ar_cond)
        return 1;

    if (temp != temp_ant || umidade != umidade_ant) {
        temp_ant = temp;
        umidade_ant = umidade;
        return 1;
    }

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

    estados.temp = temp;
    estados.umidade = umidade;
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
    StateSensor estados_iniciais;
    memset(&estados_iniciais, 0, sizeof(estados_iniciais));
    cJSON* json = buildJson(estados_iniciais, info.porta_servidor_distribuido);
    char* mensagem = cJSON_Print(json);

    char* nome_andar = cJSON_Print(buildJsonToName(info.nome));
    while (envia(info.ip_servidor_central, info.porta_servidor_central, mensagem) == -1) {
        printf("Aguardando conectar-se ao servidor_central\n");
        sleep(2);
    }

    envia(info.ip_servidor_central, info.porta_servidor_central, nome_andar);
}

void altera_todos_sensore(int value) {
    for (int i = 0; i < info.qntd_outputs; i++) {
        if (strcmp(info.outputs[i].type, "aspersor") != 0)
            write_sensor_value(info.outputs[i].gpio, value);
    }
}


void* observa_sensores(void* args) {
    int cont = 0;
    StateSensor estados_anteriores = *(StateSensor*)args;
    StateSensor estados;

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

void* obter_temp_umidade(void* args) {
    dht22Data dados_dht22;
    temp_ant = 0;
    umidade_ant = 0;
    int sensor_dht22 = encontra_gpio(info.sensores, 1, "dht22", 1);
    while (1) {
        dados_dht22 = get_dht_data(sensor_dht22);
        temp = dados_dht22.temp;
        umidade = dados_dht22.umi;
    }

}



int main(int argc, char** argv) {

    if (argc != 2) {
        printf("make run FILE=<arquivo_entrada>\n");
        return 0;
    }

    if (parse(argv[1]) == -1)
        return 0;

    signal(SIGINT, trata_sinal);

    info = getJSONData();
    initWiringPi();

    StateSensor args;
    cJSON* json;
    JSONMessage solicitacao;


    envia_estados_iniciais();

    pthread_create(&(thread_cliente), NULL, &observa_sensores, &args);
    pthread_create(&(thread_dht22), NULL, &obter_temp_umidade, &args);
    inicializaEscuta(info.porta_servidor_distribuido);

    while (1) {
        json = obterMensagem();
        solicitacao = parseMessage(json);
        int gpio = encontra_gpio(info.outputs, info.qntd_outputs, solicitacao.sensor, solicitacao.numero);

        if (gpio != -1)
            write_sensor_value(gpio, solicitacao.comand);
        else
            altera_todos_sensore(solicitacao.comand);

    }

    finalizaEscuta();

    return 0;
}
