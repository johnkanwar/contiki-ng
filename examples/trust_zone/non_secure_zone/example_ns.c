#if (__ARM_FEATURE_CMSE & 1) == 0
#error "Need ARMv8-M security extensions"
#elif (__ARM_FEATURE_CMSE & 2) != 0
#error "Don't compile with --cmse in order to make the build non-secure"
#endif


#include "contiki.h"

#include <stdio.h> /* For printf() */

/*nRF includes*/
#include "hal/nrf_gpio.h"
#include "hal/nrf_reset.h"
#include "hal/nrf_spu.h"
#include "dev/leds.h"
#include "dev/button-hal.h"
#include "nrf5340_application.h" //Includes core_cm33
#include "system_nrf5340_application.h"
#include "tz_context.h"
//#include <arm_cmse.h>     // CMSE definitions
//#include "SPM_lite.h"
//#include "example_ns.h"
#include "veneer_table.h"


///*---------------------------------------------------------------------------*/
//void
//secure_init(void)
//{
//PROCESS_NAME(example_ns_process);
//  process_start(&example_ns_process, NULL);
//}
///*---------------------------------------------------------------------------*/
//int
//get_int_nr(int val){
//    return val;
//}

/*---------------------------------------------------------------------------*/
PROCESS(example_ns_process, "example ns process");
AUTOSTART_PROCESSES(&example_ns_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_ns_process, ev, data)
{

  static struct etimer timer;
  static int i = 0;
  //static int id_nr = 32;
  //static tz_nonsecure_setup_conf_t nonsec_conf;
  PROCESS_BEGIN();
  //secure_init();
  etimer_set(&timer, CLOCK_SECOND * 1);





    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
	printf("NON SECURE\n");

    //SCB->VTOR = VECT_TAB_BASE_ADDRESS | VECT_TAB_OFFSET;

//    printf("From non-secure world, calling non-secure callable call %d \n",secure_funcX());
if(1){
    printf("From non-secure world, calling non-secure callable %d \n",secure_func_entryX(200));
}
    //	printf("From non-secure world, calling secure function %d \n", secure_func_non_objX());


  while(1) {

    PROCESS_YIELD();


    printf("Hello, world %d\n", i);
    i++;
    /* Wait for the periodic timer to expire and then restart the timer. */
    //PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    //etimer_reset(&timer);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
