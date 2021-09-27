/*
 * Secure example.
 * This code is compiled into a secure executable
 * with help of -mcmse and co.
*/


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
//#include "tz_context.h"
#include <arm_cmse.h>     // CMSE definitions

//#include "arm_cmse.h"
#include "veneer_table.h"
//#include "subdir/secure_world.h"

/*
 * The linker file
 * contiki-ng/arch/cpu/nrf/lib/nrfx/mdk/nrf_common.ld
 *
 * */


/*---------------------------------------------------------------------------*/
PROCESS(example_s_process, "Example s process");
AUTOSTART_PROCESSES(&example_s_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_s_process, ev, data)
{


//  static int test_var = 0;
  static struct etimer timer;
  static int i = 0;
  static int id_nr = 32;
  PROCESS_BEGIN();

//	sau_and_idau_cfg();
//    secure_init();

  /* Setup a periodic timer that expires after 1 seconds. */
  etimer_set(&timer, CLOCK_SECOND * 1);


    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
	printf("Start\n");
    printf("__ARM_FEATURE_CMSE %d \n",__ARM_FEATURE_CMSE);
 	printf("This is text printed from secure world \n");
	//printf("Module is : %d \n", BUILD_WITH_SECURE);
    printf("Printing the NSC func entry: %d \n",secure_func_entryX(10));


    /*nRF non-secure/secure setups not used.*/

   /*Setting the lock_conf to true makes it not readable*/
//    nrf_spu_flashregion_set(NRF_SPU,id_nr,true,0,true);

//	NRF_SPU->FLASHREGION[id_nr].PERM &= ~(1U << 1);
//nrf_spu_flashnsc_set(NRF_SPU,0,NRF_SPU_NSC_SIZE_4096B,id_nr,false);
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
