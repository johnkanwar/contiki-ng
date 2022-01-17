#include "contiki.h"
#include <stdio.h> /* For printf() */

#include "dev/leds.h"
#include "dev/etc/rgb-led/rgb-led.h"
// #include "../secure_zone/test.h"

// int secure_test_func(void);
/*---------------------------------------------------------------------------*/
PROCESS(example_ns_process, "example ns process");
AUTOSTART_PROCESSES(&example_ns_process); //&example_ns_process
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_ns_process, ev, data)
{
  static struct etimer timer;

  PROCESS_BEGIN();

  printf("Non secure world\n");
  leds_single_toggle(3);

  /*Test non-secure callable function*/
  // printf("Calling secure function %d \n", secure_test_func());

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
