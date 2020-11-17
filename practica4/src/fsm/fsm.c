#include "fsm.h"

void initial_foo(int *seconds, fsm_event *ev) {
    *seconds = 0;
    printChrono(*seconds, *ev);
}


void running_foo(int *seconds, fsm_event *ev) {
    (*seconds)++;
    printChrono(*seconds, *ev);
}


void stopped_foo(int *seconds, fsm_event *ev) {
    printChrono(*seconds, *ev);
}


void printChrono(int seconds, fsm_event ev) {
    if (ev != one_sec)
        return;

    int ss, mm;

    ss = seconds % 60;
    mm = seconds / 60;

    printf("%d:%d\n", mm, ss);
}
