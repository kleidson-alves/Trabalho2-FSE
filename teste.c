#include <stdio.h>

typedef struct abc
{
    int a;
    int b;
}abc;

int main() {
    abc a;
    abc b;

    a.a = 1;
    a.b = 2;

    b.a = 3;
    b.b = 4;
    return 0;
}
