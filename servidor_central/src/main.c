#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include <pthread.h>

#include "servidor_tcp.h"
#include "cJSON.h"

void* servidor_escuta(void* args) {
    unsigned short porta = *(unsigned short*)args;
    cJSON* json = escuta(porta);
    char* mensagem = cJSON_Print(json);
    printf("%s\n", mensagem);
}

int main(void) {

    pthread_t t1;
    unsigned short porta = 10051;

    pthread_create(&t1, NULL, servidor_escuta, &porta);

    // int c = 0;
    // initscr();
    // curs_set(0);

    while (1) {

    }
    // endwin();
    return 0;
}