/*
 * Secure example.
 */

#include "contiki.h"
#include <stdio.h> /* For printf() */

#include <arm_cmse.h> // CMSE definitions
// #include "veneer_table.h"
#include "target_cfg.h"

// #include <cortex-m/tz.h>
#include "dev/leds.h"
#include "dev/etc/rgb-led/rgb-led.h"
#include "region_defs.h"

typedef void __attribute__((cmse_nonsecure_call)) (*tz_ns_func_ptr_t)(void);
#define TZ_NONSECURE_FUNC_PTR_DECLARE(fptr) tz_ns_func_ptr_t fptr

#define TZ_NONSECURE_FUNC_PTR_CREATE(fptr) \
  ((tz_ns_func_ptr_t)(cmse_nsfptr_create(fptr)))

/*---------------------------------------------------------------------------*/
PROCESS(example_s_process, "Example s process");
AUTOSTART_PROCESSES(&example_s_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_s_process, ev, data)
{

  PROCESS_BEGIN();

  printf("Secure world\n");

  /*SPM example*/
  if (SPM_EXAMPLE){
    printf("Running example SPM_EXAMPLE\n");

    if (enable_fault_handlers() == TFM_PLAT_ERR_SUCCESS){
      printf("Succsess\n");
    }else{
      printf("Failure\n");
    }

    /*Set flash and RAM secure,
      set non-secure parition non-secure for both flash and RAM
      Set all peripherials non-secure*/

    sau_and_idau_cfg();
    non_secure_configuration();
    unsigned int *vtor_ns = (unsigned int *)NS_CODE_START;
    printf("NS IMAGE AT 0x%x \n", (unsigned int)vtor_ns);
    printf("NS MSP AT 0x%x \n", vtor_ns[0]);
    printf("NS reset vector AT 0x%x \n", vtor_ns[1]);

    /*Configure non-secure stack*/
    tz_nonsecure_setup_conf_t spm_ns_conf = {
        .vtor_ns = (unsigned int)vtor_ns,
        .msp_ns = vtor_ns[0],
        .psp_ns = 0,
        .control_ns.npriv = 0, /* Privileged mode*/
        .control_ns.spsel = 0  /*Use MSP in Thread mode*/

    };

    tz_nonsecure_state_setup(&spm_ns_conf); /*Implicit declaration of func*/
    TZ_NONSECURE_FUNC_PTR_DECLARE(reset_ns);
    reset_ns = TZ_NONSECURE_FUNC_PTR_CREATE(vtor_ns[1]);

    if (cmse_is_nsfptr(reset_ns)){
      printf("SPM: prepare to jump to Non-Secure image. \n");
      sau_and_idau_cfg();
      spu_periph_config_uarte();

      __DSB();
      __ISB();
      reset_ns();
    }else{
      printf("ERROR: WRONG POINTER TYPE\n");
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
