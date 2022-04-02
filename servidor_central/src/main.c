#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#include "servidor_tcp.h"
#include "cliente_tcp.h"
#include "cJSON.h"
#include "cJSON_interface.h"


#define RED 1
#define GREEN 2
#define BLUE 3
#define DEFAULT 4

cJSON* json;
JSONData* estados_sensores;

char** andares;
int* qntd_pessoas;
int pessoas_predio = 0;
int terreo;
int qntd_andares = 0;
int andar_atual = 0;
int alarme = 0;

int liga_desliga(int estado) {
    if (estado == 1)
        return 0;

    return 1;
}

int verifica_nova_conexao(JSONData json_data) {
    int nova = 1;
    for (int i = 0; i < qntd_andares; i++) {
        if (estados_sensores[i].distribuido_porta == json_data.distribuido_porta) {
            return 0;
        }
    }
    return nova;
}

void atualiza_andar(JSONData json_data) {
    for (int i = 0; i < qntd_andares; i++) {
        if (estados_sensores[i].distribuido_porta == json_data.distribuido_porta) {
            estados_sensores[i] = json_data;
            return;
        }
    }
}

int  verifica_andar(JSONData json_data) {
    for (int i = 0; i < qntd_andares; i++) {
        if (estados_sensores[i].distribuido_porta == json_data.distribuido_porta) {
            return i;
        }
    }

    return -1;
}

void trata_retorno(JSONMessage mensagem) {
    if (strcmp("erro", mensagem.sensor) == 0) {
        attron(COLOR_PAIR(RED));
        mvprintw(15, 50, "Não foi possível comunicar com o servidor distribuído");
        attroff(COLOR_PAIR(RED));
    }
    else {
        attron(COLOR_PAIR(GREEN));
        if (mensagem.comand == 1)
            mvprintw(15, 50, "ligando %s . . .", mensagem.sensor);
        else
            mvprintw(15, 50, "desligando ligando %s . . .", mensagem.sensor);

        attroff(COLOR_PAIR(RED));
    }
}



void* servidor_escuta(void* args) {
    unsigned short porta = *(unsigned short*)args;
    JSONData info;

    int estado_anterior_entrada = 0;
    int estado_anterior_saida = 0;
    int andar_json;

    inicializaEscuta(porta);

    while (1) {
        json = obterMensagem();
        info = parseJson(json);
        if (qntd_andares == 0 || verifica_nova_conexao(info)) {
            cJSON* json_nome = obterMensagem();
            char* nome_andar = getFloorName(json_nome);
            if (strcmp("Térreo", nome_andar) == 0)
                terreo = qntd_andares;
            estados_sensores = realloc(estados_sensores, (qntd_andares + 1) * sizeof(JSONData));
            andares = realloc(andares, (qntd_andares + 1) * sizeof(andares));
            qntd_pessoas = realloc(qntd_pessoas, (qntd_andares + 1) * sizeof(int));

            andares[qntd_andares] = nome_andar;
            qntd_pessoas[qntd_andares] = 0;
            estados_sensores[qntd_andares] = info;

            qntd_andares++;
        }
        andar_json = verifica_andar(info);

        estado_anterior_entrada = estados_sensores[andar_json].estado_entrada;
        estado_anterior_saida = estados_sensores[andar_json].estado_saida;

        if (strcmp(andares[andar_json], "Térreo") == 0) {
            if (info.estado_entrada == 1 && estado_anterior_entrada == 0)
                pessoas_predio++;
            if (info.estado_saida == 1 && estado_anterior_saida == 0)
                pessoas_predio--;
        }
        else {
            if (info.estado_entrada == 1 && estado_anterior_entrada == 0)
                qntd_pessoas[andar_json]++;
            if (info.estado_saida == 1 && estado_anterior_saida == 0)
                qntd_pessoas[andar_json]--;
        }

        int soma = 0;
        for (int i = 0; i < qntd_andares; i++) {
            if (strcmp(andares[i], "Térreo") != 0)
                soma += qntd_pessoas[i];
        }
        qntd_pessoas[terreo] = pessoas_predio - soma;

        atualiza_andar(info);
    }

    finalizaEscuta();
}

void* aguarda_comando_usuario(void* args) {
    int* exit = (int*)args;
    char* message;
    int i = andar_atual;

    while (1) {
        char entrada_usuario = getch();
        message = NULL;
        switch (entrada_usuario) {
        case '1':
            message = buildMessage("lampada", 1, liga_desliga(estados_sensores[andar_atual].lampada1));
            break;
        case '2':
            message = buildMessage("lampada", 2, liga_desliga(estados_sensores[andar_atual].lampada2));
            break;
        case '3':
            message = buildMessage("lampada", 3, liga_desliga(estados_sensores[andar_atual].lampada_corredor));
            break;
        case '4':
            message = buildMessage("ar-condicionado", 1, liga_desliga(estados_sensores[andar_atual].ar_cond));
            break;
        case '5':
            message = buildMessage("aspersor", 1, liga_desliga(estados_sensores[andar_atual].aspersor));
            break;

        case ' ':
            i++;
            if (i == qntd_andares)
                i = 0;
            andar_atual = i;
            break;

        case 'q':
            *exit = 1;
            break;
        }
        if (message != NULL) {
            JSONMessage json_message;
            char* return_message;
            return_message = envia("192.168.0.38", estados_sensores[andar_atual].distribuido_porta, message);
            json_message = parseMessage(return_message);
            trata_retorno(json_message);
        }
    }
}



void seleciona_cor(int estado) {
    if (estado == 1) {
        attron(COLOR_PAIR(GREEN));
    }
    else
        attron(COLOR_PAIR(RED));
}

void apresenta_info() {
    attron(COLOR_PAIR(DEFAULT));
    mvprintw(0, 0, "------------- Informações %s -------------", andares[andar_atual]);
    attroff(COLOR_PAIR(DEFAULT));

    attron(COLOR_PAIR(BLUE));
    mvprintw(2, 0, "Pessoas no prédio: %d", pessoas_predio);
    mvprintw(3, 0, "Pessoas no andar: %d", qntd_pessoas[andar_atual]);
    attroff(COLOR_PAIR(BLUE));

    seleciona_cor(estados_sensores[andar_atual].presenca);
    mvprintw(2, 40, "Sensor de presença");
    seleciona_cor(estados_sensores[andar_atual].fumaca);
    mvprintw(3, 40, "Sensor de fumaça");
    seleciona_cor(estados_sensores[andar_atual].janela01);
    mvprintw(4, 40, "Janela 1");
    seleciona_cor(estados_sensores[andar_atual].janela02);
    mvprintw(5, 40, "Janela 2");

    if (strcmp(andares[andar_atual], "Térreo") == 0) {
        seleciona_cor(estados_sensores[andar_atual].porta);
        mvprintw(6, 40, "Porta de Entrada");
    }


}

void menu_comandos() {
    attroff(COLOR_PAIR(GREEN));
    int row_init = 8;
    mvprintw(row_init, 0, "-------------------- COMANDOS --------------------");

    seleciona_cor(estados_sensores[andar_atual].lampada1);
    mvprintw(row_init + 2, 0, "[1] Lampada Sala 1");

    seleciona_cor(estados_sensores[andar_atual].lampada2);
    mvprintw(row_init + 3, 0, "[2] Lampada  Sala 2");

    seleciona_cor(estados_sensores[andar_atual].lampada_corredor);
    mvprintw(row_init + 4, 0, "[3] Lampada Corredor");

    seleciona_cor(estados_sensores[andar_atual].ar_cond);
    mvprintw(row_init + 2, 30, "[4] Ar Condicionado");

    if (strcmp(andares[andar_atual], "Térreo") == 0) {
        seleciona_cor(estados_sensores[andar_atual].aspersor);
        mvprintw(row_init + 3, 30, "[5] Aspersor");
        seleciona_cor(alarme);
        mvprintw(row_init + 4, 30, "[6] Alarme\n");
    }

    attroff(COLOR_PAIR(GREEN));
    attroff(COLOR_PAIR(RED));

    if (qntd_andares > 0)
        mvprintw(row_init + 10, 10, "Pressione espaço para trocar de andar");

}


int main(void) {
    pthread_t t1, thread_menu;

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


    while (qntd_andares == 0) {
        mvprintw(1, 5, "Aguardando servidor distribuído");
        refresh();
        sleep(1);
    }

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