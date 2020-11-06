#include "fsm.h"

void initial_foo(int *seconds) {
    *seconds = 0;
    printChrono(*seconds);
}


void running_foo(int *seconds) {
    (*seconds)++;
    printChrono(*seconds);
}


void stopped_foo(int *seconds) {
    printChrono(*seconds);
}


void printChrono(int seconds) {
    int ss, mm;

    ss = seconds % 60;
    mm = seconds / 60;

    printf("%d:%d\n", mm, ss);
}
