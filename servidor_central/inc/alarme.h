#ifndef ALARME_H
#define ALARME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

#include "cJSON_interface.h"

#define LIGA 1
#define DESLIGA 1


void altera_estado_alarme(char* nome, int estado);
void dispara();
void controla_alarme_seguranca(JSONData* estados, int qntd_andares);
int controla_alarme_fumaca(int estado_fumaca);
int obter_estado_alarme();


#endif