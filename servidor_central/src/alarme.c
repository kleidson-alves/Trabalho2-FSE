#include "alarme.h"

int alarme_seguranca = 0;
int alarme_fumaca = 0;

void dispara() {
    if (alarme_seguranca || alarme_fumaca) {
        attron(COLOR_PAIR(1));
        mvprintw(0, 30, "Alarme disparado");
        beep();
    }
}

int controla_alarme_fumaca(int estado_fumaca) {
    alarme_fumaca = estado_fumaca;
    return alarme_fumaca;
}

void controla_alarme_seguranca(JSONData* estados, int qntd_andares) {
    for (int i = 0; i < qntd_andares; i++) {
        if (!estados[i].janela01 || !estados[i].janela02 || estados[i].porta == 0 || !estados[i].presenca) {
            alarme_seguranca = 1;
            break;
        }
    }
}

int obter_estado_alarme() {
    return alarme_seguranca;
}

