#include "fsm.h"

void initial_foo(int *seconds) {
    printf("ini\n");
    *seconds = 0;
    printChrono(*seconds);
}


void running_foo(int *seconds) {
    printf("running\n");
    (*seconds)++;
    printChrono(*seconds);
}


void stopped_foo(int *seconds) {
    printf("stoppped\n");
    printChrono(*seconds);
}


void printChrono(int seconds) {
    int ss, mm;

    ss = seconds % 60;
    mm = seconds / 60;

    printf("%d:%d\n", mm, ss);
}
