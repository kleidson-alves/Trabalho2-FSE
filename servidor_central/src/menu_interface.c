#include "menu_interface.h"

char** andares;
int qntd_andares, pessoas_predio, andar_atual;
int* qntd_pessoas;
JSONData* estados_sensores;

void inicializa_cores() {
    init_pair(DEFAULT, COLOR_WHITE, COLOR_BLACK);
    init_pair(RED, COLOR_RED, COLOR_BLACK);
    init_pair(GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(BLUE, COLOR_BLUE, COLOR_BLACK);

}
void seleciona_cor(int estado) {
    if (estado == 1) {
        attron(COLOR_PAIR(GREEN));
    }
    else
        attron(COLOR_PAIR(RED));
}

void apresenta_info_geral() {
    int row = 1;
    attron(COLOR_PAIR(DEFAULT));

    mvprintw(0, 0, "Pessoas no prédio: %d", pessoas_predio);
    for (int i = 0; i < qntd_andares; i++)
        mvprintw(i + row, 0, "Pessoas no %s: %d", andares[i], qntd_pessoas[i]);
    attroff(COLOR_PAIR(DEFAULT));

}


void apresenta_info() {
    int row_init = qntd_andares + 3;
    int ident_column = 30;
    attron(COLOR_PAIR(DEFAULT));
    mvprintw(row_init, 0, "---------------- Informações %s ----------------", andares[andar_atual]);
    attroff(COLOR_PAIR(DEFAULT));

    attron(COLOR_PAIR(BLUE));
    mvprintw(row_init + 2, 0, "Pessoas no andar: %d", qntd_pessoas[andar_atual]);

    if (estados_sensores[andar_atual].temp == 0) {
        attron(COLOR_PAIR(DEFAULT));
        mvprintw(row_init + 3, 0, "Carregando temperatura");
        mvprintw(row_init + 4, 2, "umidade . . .");
        attroff(COLOR_PAIR(DEFAULT));
    }
    else {
        mvprintw(row_init + 3, 0, "Umidade: %.1lf%%", estados_sensores[andar_atual].umidade);
        mvprintw(row_init + 4, 0, "Temperatura: %.1lf", estados_sensores[andar_atual].temp);
    }
    attroff(COLOR_PAIR(BLUE));

    seleciona_cor(estados_sensores[andar_atual].presenca);
    mvprintw(row_init + 2, ident_column, "Sensor de presença");
    seleciona_cor(estados_sensores[andar_atual].fumaca);
    mvprintw(row_init + 3, ident_column, "Sensor de fumaça");
    seleciona_cor(estados_sensores[andar_atual].janela01);
    mvprintw(row_init + 4, ident_column, "Janela 1");
    seleciona_cor(estados_sensores[andar_atual].janela02);
    mvprintw(row_init + 5, ident_column, "Janela 2");

    if (strcmp(andares[andar_atual], "Térreo") == 0) {
        seleciona_cor(estados_sensores[andar_atual].porta);
        mvprintw(row_init + 6, ident_column, "Porta de Entrada");
    }
}


void menu_comandos() {
    attroff(COLOR_PAIR(GREEN));
    int row_init = qntd_andares + 12;
    int column_init = 0;
    int column_ident = 30;
    mvprintw(row_init, column_init, "-------------------- COMANDOS --------------------");

    seleciona_cor(estados_sensores[andar_atual].lampada1);
    mvprintw(row_init + 2, column_init, "[1] Lâmpada Sala 1");

    seleciona_cor(estados_sensores[andar_atual].lampada2);
    mvprintw(row_init + 3, column_init, "[2] Lâmpada  Sala 2");

    seleciona_cor(estados_sensores[andar_atual].lampada_corredor);
    mvprintw(row_init + 4, column_init, "[3] Lâmpada Corredor");

    seleciona_cor(estados_sensores[andar_atual].ar_cond);
    mvprintw(row_init + 2, column_ident, "[4] Ar Condicionado");

    if (strcmp(andares[andar_atual], "Térreo") == 0) {
        seleciona_cor(obter_estado_alarme());
        mvprintw(row_init + 3, column_ident, "[5] Alarme\n");
    }

    attroff(COLOR_PAIR(GREEN));
    attroff(COLOR_PAIR(RED));

    row_init += 6;

    mvprintw(row_init, 0, "-------------- COMANDOS ESPECIAIS --------------");
    mvprintw(row_init + 2, column_init, "[l] ligar todos os sensores do andar");
    mvprintw(row_init + 3, column_init, "[d] desligar todos os sensores do andar");
    mvprintw(row_init + 4, column_init, "[o] ligar todos os sensores do prédio");
    mvprintw(row_init + 5, column_init, "[i] desligar todos os sensores do prédio");

    mvprintw(row_init + 6, column_init, "[q] sair");

    if (qntd_andares > 1)
        mvprintw(row_init + 10, 8, "Pressione espaço para trocar de andar");

}

void menu(JSONData* estados, char** _andares, int* qntds, int qntd_andar, int total_pessoas, int andar) {
    inicializa_cores();
    estados_sensores = estados;
    andares = _andares;
    qntd_pessoas = qntds;
    qntd_andares = qntd_andar;
    pessoas_predio = total_pessoas;
    andar_atual = andar;
    apresenta_info_geral();
    apresenta_info();
    menu_comandos();
}