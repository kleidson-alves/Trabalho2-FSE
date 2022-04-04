#include "alarme.h"

int alarme_seguranca = 0;
int alarme_fumaca = 0;

void dispara() {
    if (alarme_seguranca || alarme_fumaca) {
        attron(COLOR_PAIR(1));
        if (alarme_fumaca && alarme_seguranca)
            mvprintw(0, 30, "Alarmes de Incêndio e Segurança disparados");
        else if (alarme_seguranca)
            mvprintw(0, 30, "Alarme de Segurança disparado");
        else
            mvprintw(0, 30, "Alarme de Incêndio disparado");

        beep();
    }
}

int controla_alarme_fumaca(int estado_fumaca) {
    alarme_fumaca = estado_fumaca;
    return alarme_fumaca;
}

void controla_alarme_seguranca(int comando) {
    alarme_seguranca = comando;
}

int obter_estado_alarme() {
    return alarme_seguranca;
}

