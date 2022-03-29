#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>

int main(void) {
    int c = 0;
    initscr();
    curs_set(0);
    while (c < 1000) {
        mvprintw(0, 0, "%d", c++);
        refresh();
        usleep(1000);
    }
    endwin();
    return 0;
}