#include <stdio.h>

#include "contiki.h"
#include "sys/etimer.h"
#include "sys/ctimer.h"

#define TIMEOUT1 4
#define TIMEOUT2 5

PROCESS(process1, "Process1");
PROCESS(process2, "Process2");
AUTOSTART_PROCESSES(&process1, &process2);

static int counter_event1;
static int counter_event2;
static struct etimer timer_etimer;
static struct ctimer timer_ctimer;
static process_event_t process1_event;
static process_event_t process2_event;

void do_callback() {
  process_post(&process1, process2_event, NULL);
  printf("Callback timer timeout\n");
  ctimer_reset(&timer_ctimer);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(process1, ev, data)
{
  static struct etimer timer_etimer;

  PROCESS_BEGIN();

  process1_event = process_alloc_event();
  etimer_set(&timer_etimer, TIMEOUT1 * CLOCK_SECOND);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER || ev == process2_event);

    if(ev == PROCESS_EVENT_TIMER){
      process_post_synch(&process2, process1_event, NULL);
      etimer_reset(&timer_etimer);
      printf("Event timer timeout\n");
    }
    else if (ev == process2_event){
      counter_event2++;
      printf("Process1 has received %i from Process2\n", counter_event2);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(process2, ev, data)
{
  PROCESS_BEGIN();

  process2_event = process_alloc_event();
  ctimer_set(&timer_ctimer, TIMEOUT2 * CLOCK_SECOND, do_callback, NULL);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == process1_event);
    counter_event1++;
    printf("Process2 has received %i from Process1\n", counter_event1);
  }

  PROCESS_END();
}