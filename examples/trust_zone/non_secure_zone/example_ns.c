#if (__ARM_FEATURE_CMSE & 1) == 0
#error "Need ARMv8-M security extensions"
#elif (__ARM_FEATURE_CMSE & 2) != 0
#error "Don't compile with --cmse "in order to make the build" non-secure"
#endif

#include "contiki.h"
#include <stdio.h> /* For printf() */

  #include "dev/leds.h"
#include "dev/etc/rgb-led/rgb-led.h"

static int non_secure_variable = 80;
const static int non_secure_variable1 = 81;
static int non_secure_variable3;
int non_secure_variable4;
non_secure_variable4 = 84;

/*---------------------------------------------------------------------------*/
PROCESS(example_ns_process, "example ns process");
AUTOSTART_PROCESSES(&example_ns_process); //&example_ns_process
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_ns_process, ev, data)
{
  non_secure_variable3 = 83;

    // leds_single_on(3);

  PROCESS_BEGIN();



  printf("NON SECURE WORLD\n");
  leds_single_on(3);

  static int non_secure_variable2 = 82;

  // printf("non_secure_variable value: %d, memory address: %p \n", non_secure_variable, &non_secure_variable);
  // printf("non_secure_variable value: %d, memory address: %p \n", non_secure_variable1, &non_secure_variable1);
  // printf("non_secure_variable value: %d, memory address: %p \n", non_secure_variable2, &non_secure_variable2);
  // printf("non_secure_variable value: %d, memory address: %p \n", non_secure_variable3, &non_secure_variable3);
  // printf("non_secure_variable value: %d, memory address: %p \n", non_secure_variable4, &non_secure_variable4);

  // while (1){
  //   // PROCESS_YIELD();

  //   printf("Hello, world non secure\n");
  // }

  /* Wait for the periodic timer to expire and then restart the timer. */
  // PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
  // etimer_reset(&timer);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
