/*
 * Secure example.
 * This code is compiled into a secure executable
 * with help of -mcmse and co.
 */

#include "contiki.h"

#include <assert.h>
#include <stdio.h> /* For printf() */
#include "defines_config.h"
/*nRF includes*/
#include "hal/nrf_gpio.h"
#include "hal/nrf_reset.h"
#include "hal/nrf_spu.h"
#include "dev/leds.h"
#include "dev/button-hal.h"
#include "nrf5340_application.h" //Includes core_cm33
#include "system_nrf5340_application.h"
//#include "tz_context.h"
#include <arm_cmse.h> // CMSE definitions
#include "spu.h"
#include "region_defs.h"
#include "veneer_table.h"
#include "target_cfg.h"
// #include "subdir/include/boot_hal.h"

/*
 * The linker file
 * contiki-ng/arch/cpu/nrf/lib/nrfx/mdk/nrf_common.ld
 *
 * */
#define LOCATE_FUNC __attribute__((__section__(".mysection")))

int LOCATE_FUNC secure_func_read_int(int bar)
{
  return bar;
}

static int secure_variable = 30;

typedef void (*funcptr_void)(void) __attribute__((cmse_nonsecure_call));

/*Specs*/
// 1 MB Flash & 512 KB RAM
// The flash memory space is divided into 64 regions of 16 KiB.  //1 ID = 8 KB
/*---------------------------------------------------------------------------*/
PROCESS(example_s_process, "Example s process");
AUTOSTART_PROCESSES(&example_s_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_s_process, ev, data)
{

  static struct etimer timer;
  static int i = 0;
  PROCESS_BEGIN();

  etimer_set(&timer, CLOCK_SECOND * 1);

  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
  printf("Start\n");
  printf("__ARM_FEATURE_CMSE %d \n", __ARM_FEATURE_CMSE);
  printf("This is text printed from secure world \n");

  /*Configuration*/
  sau_and_idau_cfg();

  if (enable_fault_handlers() == TFM_PLAT_ERR_SUCCESS){
    printf("Succsess\n");
  }else{
    printf("Failure\n");
  }
  if (spu_init_cfg() == TFM_PLAT_ERR_SUCCESS){
    printf("Succsess 1\n");
  }else{
    printf("Failure 1\n");
  }
  spu_peripheral_config_secure_all(256 - 1, 1);

  if (nvic_interrupt_enable() == TFM_PLAT_ERR_SUCCESS){
    printf("Succsess 2\n");
  }else{
    printf("Failure 2\n");
  }
  TZ_SAU_Enable();
  // SAU->CTRL &= ~(SAU_CTRL_ALLNS_Msk);

  printf("Secure_variable value: %d, memory address: %p \n", secure_variable, &secure_variable);
  printf("Memory addres of secure_func_read_int %p\n", &secure_func_read_int);
  printf("Calling the secure variable from SECURE zone: %d \n", secure_variable);
  printf("Calling the secure variable from NON secure zone: %d \n", secure_func_read_int(secure_variable));

  /*Shouldn't be able to touch the peripherials if they are secure*/
  if (enable_fault_handlers() == TFM_PLAT_ERR_SUCCESS){
    printf("Succsess 1\n");
  }else{
    printf("Failure 1\n");
  }
  printf("End of main \n");
  while (1)
  {

    PROCESS_YIELD();

    printf("Hello, world %d\n", i);
    i++;
    /* Wait for the periodic timer to expire and then restart the timer. */
    // PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    // etimer_reset(&timer);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/