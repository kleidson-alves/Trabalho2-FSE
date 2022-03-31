#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include "servidor_tcp.h"
#include "cliente_tcp.h"
#include "cJSON.h"
#include "cJSON_interface.h"

#define LAMP_1 0
#define LAMP_2 1
#define LAMP_CORREDOR 2
#define AR_COND 3
#define ASPERSOR 4
#define MAX_ANDARES 2

#define RED 1
#define GREEN 2
#define BLUE 3
#define DEFAULT 4



cJSON* json;
JSONData info;

int qntd_pessoas = 0;
int estados_sensores[MAX_ANDARES][5];
int andar_atual = 1;

int liga_desliga(int sensor) {
    for (int i = 0; i < 5;i++) {
        if (i == sensor) {
            if (estados_sensores[andar_atual][i] == 1)
                return 0;
            else
                return 1;
        }
    }

    return -1;
}

int busca_sensor(char* nome, int pos) {
    if (strcmp(nome, "lampada") == 0) {
        if (pos == 1)
            return LAMP_1;
        else if (pos == 2)
            return LAMP_2;
        else
            return LAMP_CORREDOR;
    }
    else if (strcmp(nome, "aspersor") == 0)
        return ASPERSOR;
    else if (strcmp(nome, "ar-condicionado") == 0)
        return AR_COND;

    return -1;
}

void trata_mensagem(JSONMessage mensagem) {
    int sensor = busca_sensor(mensagem.sensor, mensagem.numero);
    if (sensor != -1) {
        estados_sensores[andar_atual][sensor] = liga_desliga(sensor);
    }
}

void* servidor_escuta(void* args) {
    unsigned short porta = *(unsigned short*)args;
    int estado_anterior_entrada = 0;
    int estado_anterior_saida = 0;
    inicializaEscuta(porta);
    while (1) {
        json = obterMensagem();
        info = parseJson(json);

        if (info.estado_entrada == 1 && estado_anterior_entrada == 0)
            qntd_pessoas++;
        if (info.estado_saida == 1 && estado_anterior_saida == 0)
            qntd_pessoas--;

        estado_anterior_entrada = info.estado_entrada;
        estado_anterior_saida = info.estado_saida;
    }
    finalizaEscuta();
}

void* aguarda_comando_usuario(void* args) {
    int* exit = (int*)args;
    char* message;

    while (1) {
        char entrada_usuario = getch();
        message = NULL;
        switch (entrada_usuario) {
        case '1':
            message = buildMessage("lampada", 1, liga_desliga(LAMP_1));
            break;
        case '2':
            message = buildMessage("lampada", 2, liga_desliga(LAMP_2));
            break;
        case '3':
            message = buildMessage("lampada", 3, liga_desliga(LAMP_CORREDOR));
            break;
        case '4':
            message = buildMessage("ar-condicionado", 1, liga_desliga(AR_COND));
            break;
        case '5':
            message = buildMessage("aspersor", 1, liga_desliga(ASPERSOR));
            break;

        case 'p':
            andar_atual++;
            break;
        case 'a':
            andar_atual--;
            break;

        case 'q':
            *exit = 1;
            break;
        }
        if (message != NULL) {
            JSONMessage json_message;
            unsigned short distribuido_porta = 10151;
            char* return_message;
            return_message = envia("192.168.0.38", distribuido_porta, message);
            json_message = parseMessage(return_message);
            trata_mensagem(json_message);
        }
    }
}



void seleciona_cor(int sensor) {
    if (estados_sensores[andar_atual][sensor] == 1) {
        attron(COLOR_PAIR(GREEN));
    }
    else
        attron(COLOR_PAIR(RED));
}

void apresenta_info() {
    attron(COLOR_PAIR(DEFAULT));
    mvprintw(0, 0, "------------- Informações %dº andar -------------", andar_atual);
    attroff(COLOR_PAIR(DEFAULT));

    attron(COLOR_PAIR(BLUE));
    mvprintw(1, 0, "Pessoas no predio: %d", qntd_pessoas);
    attroff(COLOR_PAIR(BLUE));
}

void menu_comandos() {
    attroff(COLOR_PAIR(GREEN));
    mvprintw(5, 0, "------------- COMANDOS -------------");

    seleciona_cor(LAMP_1);
    mvprintw(6, 0, "1 -  Lampada Sala 1");

    seleciona_cor(LAMP_2);
    mvprintw(7, 0, "2 - Lampada  Sala 2");

    seleciona_cor(LAMP_CORREDOR);
    mvprintw(8, 0, "3 -  Lampada Corredor");

    seleciona_cor(AR_COND);
    mvprintw(6, 30, "4 - Ar Condicionado");

    if (andar_atual == 1) {
        seleciona_cor(ASPERSOR);
        mvprintw(7, 0, "5 -  Aspersor");
        mvprintw(8, 30, "6 -  Alarme\n");
    }
    if (andar_atual != 1)
        mvprintw(13, 0, "a - <");

    if (andar_atual != MAX_ANDARES)
        mvprintw(13, 30, "p - >");
}


int main(void) {
    pthread_t t1, thread_menu;
    memset(&estados_sensores, 0, sizeof(estados_sensores));

    unsigned short porta = 10051;
    int exit = 0;

    initscr();
    curs_set(0);
    noecho();
    start_color();

    init_pair(DEFAULT, COLOR_WHITE, COLOR_BLACK);
    init_pair(RED, COLOR_RED, COLOR_BLACK);
    init_pair(GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(BLUE, COLOR_BLUE, COLOR_BLACK);

    pthread_create(&t1, NULL, servidor_escuta, &porta);
    pthread_create(&thread_menu, NULL, aguarda_comando_usuario, &exit);

    while (1) {
        clear();
        apresenta_info();
        menu_comandos();
        refresh();
        sleep(1);

        if (exit)
            break;;
    }
    endwin();
    return 0;
}