#ifndef _FSM_
#define _FSM_

#include <stdio.h>

#define N_STATES 3
#define N_EVENTS 3

typedef enum { initial, running, stopped } fsm_state;
typedef enum { one_sec, start_stop, reset } fsm_event;

void initial_foo(int *, fsm_event *);
void running_foo(int *, fsm_event *);
void stopped_foo(int *, fsm_event *);

/* array and enum below must be in sync! */
static void (* state_foo[])(int *) = { initial_foo, running_foo, stopped_foo };

static fsm_state state_transitions[N_STATES][N_EVENTS] = {
    {initial, running, initial},//initial [one_sec, start_stop, reset]
    {running, stopped, initial},//running [one_sec, start_stop, reset]
    {stopped, running, initial} //stopped [one_sec, start_stop, reset]
};

void printChrono(int seconds, fsm_event ev);

#endif