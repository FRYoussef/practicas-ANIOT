#ifndef _FSM_
#define _FSM_

#include "common.h"

#define N_STATES 3
#define N_EVENTS 3

/* array and enum below must be in sync! */
void (* state_foo[])(int *) = { initial_foo, running_foo, stopped_foo };
enum fsm_state { initial, running, stopped };
enum fsm_event { one_sec, start_stop, reset };

static enum fsm_state state_transitions[N_STATES][N_EVENTS] = {
    {initial, running, initial},//initial [one_sec, start_stop, reset]
    {running, stopped, initial},//running [one_sec, start_stop, reset]
    {stopped, running, initial} //stopped [one_sec, start_stop, reset]
};

void initial_foo(int *);
void running_foo(int *);
void stopped_foo(int *);

#endif