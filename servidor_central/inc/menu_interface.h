#ifndef MENU_H
#define MENU_H

#include <ncurses.h>
#include <string.h>
#include "alarme.h"
#include "cJSON_interface.h"

#define RED 1
#define GREEN 2
#define BLUE 3
#define DEFAULT 4

void inicializa_cores();
void apresenta_info_geral();
void seleciona_cor(int estado);
void apresenta_info();
void menu_comandos();
void menu(JSONData* estados, char** _andares, int* qntds, int qntd_andar, int total_pessoas, int andar, int alarme);

#endif