#include <stdio.h>

void f(int i);

void f1(int x) {
    printf("f1 ");
    f(4);
}

void f(int i) {
    if (i == 10) return;
    printf("%d", i);
    f(i + 1);
    f1(3);
}

int main() { f(3); }
