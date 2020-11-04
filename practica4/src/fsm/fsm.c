#include "fsm.h"

void initial_foo(int *seconds) {
    seconds = 0;
    printChrono(seconds);
}


void running_foo(int *seconds) {
    seconds++;
    printChrono(seconds);
}


void stopped_foo(int *seconds) {
    printChrono(seconds);
}
