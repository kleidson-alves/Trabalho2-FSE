#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include "servidor_tcp.h"
#include "cliente_tcp.h"
#include "cJSON.h"
#include "cJSON_interface.h"
#include "alarme.h"
#include "menu_interface.h"


cJSON* json;
JSONData* estados_sensores;
pthread_t t1, thread_menu;

char** andares;
int* qntd_pessoas;
int terreo;
int pessoas_predio = 0;
int qntd_andares = 0;
int andar_atual = 0;

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
        mvprintw(0, 30, "Não foi possível comunicar com o servidor distribuído");
        attroff(COLOR_PAIR(RED));
    }
    else {
        attron(COLOR_PAIR(GREEN));
        if (mensagem.comand == 1)
            mvprintw(0, 30, "ligando %s . . .", mensagem.sensor);
        else
            mvprintw(0, 50, "desligando %s . . .", mensagem.sensor);

        attroff(COLOR_PAIR(GREEN));
    }
}

void envia_mensagem(char* mensagem, unsigned short porta) {
    JSONMessage mensagem_json;
    char* mensgaem_retorno;
    mensgaem_retorno = envia("192.168.0.38", estados_sensores[andar_atual].distribuido_porta, mensagem);
    mensagem_json = parseMessage(mensgaem_retorno);
    trata_retorno(mensagem_json);
}

void transmissao_predio(int comand) {
    char* message = buildMessage("todos", 1, 1);
    for (int i = 0; i < qntd_andares; i++)
        envia_mensagem(message, estados_sensores[i].distribuido_porta);
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

        if (info.fumaca != estados_sensores[andar_atual].fumaca) {
            char* mensagem;
            mensagem = buildMessage("aspersor", 1, controla_alarme_fumaca(info.fumaca));
            envia_mensagem(mensagem, estados_sensores[terreo].distribuido_porta);
        }

        atualiza_andar(info);
    }

    finalizaEscuta();
}

void* aguarda_comando_usuario(void* args) {
    int* exit = (int*)args;
    char* message;
    int altera_andar = andar_atual;

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
            controla_alarme_seguranca(estados_sensores, qntd_andares);
            break;

        case 'q':
            *exit = 1;
            break;

        case 'l':
            message = buildMessage("todos", 1, LIGA);
            break;

        case 'd':
            message = buildMessage("todos", 1, DESLIGA);
            break;
        case 'o':
            transmissao_predio(LIGA);
            break;
        case 'i':
            transmissao_predio(DESLIGA);
            break;

        case ' ':
            altera_andar++;
            if (altera_andar == qntd_andares)
                altera_andar = 0;
            andar_atual = altera_andar;
            break;

        }
        if (message != NULL) {
            envia_mensagem(message, estados_sensores[andar_atual].distribuido_porta);
        }
    }
}

// void seleciona_cor(int estado) {
//     if (estado == 1) {
//         attron(COLOR_PAIR(GREEN));
//     }
//     else
//         attron(COLOR_PAIR(RED));
// }

// void apresenta_info_geral() {
//     int row = 1;
//     attron(COLOR_PAIR(DEFAULT));

//     mvprintw(0, 0, "Pessoas no prédio: %d", pessoas_predio);
//     for (int i = 0; i < qntd_andares; i++)
//         mvprintw(i + row, 0, "Pessoas no %s: %d", andares[i], qntd_pessoas[i]);
//     attroff(COLOR_PAIR(DEFAULT));

// }

// void apresenta_info() {
//     int row_init = qntd_andares + 3;
//     int ident_column = 30;
//     attron(COLOR_PAIR(DEFAULT));
//     mvprintw(row_init, 0, "---------------- Informações %s ----------------", andares[andar_atual]);
//     attroff(COLOR_PAIR(DEFAULT));

//     attron(COLOR_PAIR(BLUE));
//     mvprintw(row_init + 2, 0, "Pessoas no andar: %d", qntd_pessoas[andar_atual]);

//     if (estados_sensores[andar_atual].temp == 0) {
//         attron(COLOR_PAIR(DEFAULT));
//         mvprintw(row_init + 3, 0, "Carregando temperatura e umidade . . .");
//         attroff(COLOR_PAIR(DEFAULT));
//     }
//     else {
//         mvprintw(row_init + 3, 0, "Umidade: %.1lf%%", estados_sensores[andar_atual].umidade);
//         mvprintw(row_init + 4, 0, "Temperatura: %.1lf", estados_sensores[andar_atual].temp);
//     }
//     attroff(COLOR_PAIR(BLUE));

//     seleciona_cor(estados_sensores[andar_atual].presenca);
//     mvprintw(row_init + 2, ident_column, "Sensor de presença");
//     seleciona_cor(estados_sensores[andar_atual].fumaca);
//     mvprintw(row_init + 3, ident_column, "Sensor de fumaça");
//     seleciona_cor(estados_sensores[andar_atual].janela01);
//     mvprintw(row_init + 4, ident_column, "Janela 1");
//     seleciona_cor(estados_sensores[andar_atual].janela02);
//     mvprintw(row_init + 5, ident_column, "Janela 2");

//     if (strcmp(andares[andar_atual], "Térreo") == 0) {
//         seleciona_cor(estados_sensores[andar_atual].porta);
//         mvprintw(row_init + 6, ident_column, "Porta de Entrada");
//     }
// }

// void menu_comandos() {
//     attroff(COLOR_PAIR(GREEN));
//     int row_init = qntd_andares + 12;
//     int column_init = 0;
//     int column_ident = 30;
//     mvprintw(row_init, column_init, "-------------------- COMANDOS --------------------");

//     seleciona_cor(estados_sensores[andar_atual].lampada1);
//     mvprintw(row_init + 2, column_init, "[1] Lâmpada Sala 1");

//     seleciona_cor(estados_sensores[andar_atual].lampada2);
//     mvprintw(row_init + 3, column_init, "[2] Lâmpada  Sala 2");

//     seleciona_cor(estados_sensores[andar_atual].lampada_corredor);
//     mvprintw(row_init + 4, column_init, "[3] Lâmpada Corredor");

//     seleciona_cor(estados_sensores[andar_atual].ar_cond);
//     mvprintw(row_init + 2, column_ident, "[4] Ar Condicionado");

//     if (strcmp(andares[andar_atual], "Térreo") == 0) {
//         seleciona_cor(obter_estado_alarme());
//         mvprintw(row_init + 3, column_ident, "[5] Alarme\n");
//     }

//     attroff(COLOR_PAIR(GREEN));
//     attroff(COLOR_PAIR(RED));

//     row_init += 6;

//     mvprintw(row_init, 0, "-------------- COMANDOS ESPECIAIS --------------");
//     mvprintw(row_init + 2, column_init, "[l] ligar todos os sensores do andar");
//     mvprintw(row_init + 3, column_init, "[d] desligar todos os sensores do andar");
//     mvprintw(row_init + 4, column_init, "[o] ligar todos os sensores do prédio");
//     mvprintw(row_init + 5, column_init, "[i] desligar todos os sensores do prédio");

//     mvprintw(row_init + 6, column_init, "[q] sair");

//     if (qntd_andares > 1)
//         mvprintw(row_init + 10, 8, "Pressione espaço para trocar de andar");

// }

void trata_sinal(int sinal) {
    endwin();
    pthread_cancel(thread_menu);
    pthread_cancel(t1);
    exit(0);
}


int main(void) {

    unsigned short porta = 10061;
    int exit = 0;

    signal(SIGINT, trata_sinal);

    initscr();
    curs_set(0);
    noecho();
    start_color();

    pthread_create(&t1, NULL, servidor_escuta, &porta);
    pthread_create(&thread_menu, NULL, aguarda_comando_usuario, &exit);

    while (qntd_andares == 0) {
        mvprintw(1, 5, "Aguardando servidor distribuído");
        refresh();
        sleep(1);
    }

    while (1) {
        clear();
        menu(estados_sensores, andares, qntd_pessoas, qntd_andares, pessoas_predio, andar_atual);
        dispara();
        refresh();
        sleep(1);

        if (exit)
            break;;
    }

    trata_sinal(exit);
    return 0;
}