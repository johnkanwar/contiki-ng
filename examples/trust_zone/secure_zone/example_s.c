/*
 * Secure example.
 */

#include "contiki.h"
#include <stdio.h> /* For printf() */

#include <arm_cmse.h> // CMSE definitions
// #include "veneer_table.h"
#include "target_cfg.h"

// #include <cortex-m/tz.h>
#include "SPM_lite.h"
#include "dev/leds.h"
#include "dev/etc/rgb-led/rgb-led.h"

static int secure_variable = 44;

#undef cmse_nsfptr_create
#define cmse_nsfptr_create(p) ((intptr_t)(p) & ~1)

typedef void (*nsfptr_t)(void) __attribute__((cmse_nonsecure_call));

nsfptr_t ns_entry;

/*Zephyr test*/
typedef void __attribute__((cmse_nonsecure_call)) (*tz_ns_func_ptr_t)(void);
#define TZ_NONSECURE_FUNC_PTR_DECLARE(fptr) tz_ns_func_ptr_t fptr

#define TZ_NONSECURE_FUNC_PTR_CREATE(fptr) \
  ((tz_ns_func_ptr_t)(cmse_nsfptr_create(fptr)))

void jump_to_ns_code(void)
{
  spu_periph_config_uarte();

  __DSB();
  __ISB();
  /* Calls the non-secure Reset_Handler to jump to the non-secure binary */
  ns_entry();
  // tfm_core_panic();
}

void configure_ns_code(void)
{
  /* SCB_NS.VTOR points to the Non-secure vector table base address */
  SCB_NS->VTOR = tfm_spm_hal_get_ns_VTOR();

  /* Setups Main stack pointer of the non-secure code */
  unsigned int ns_msp = tfm_spm_hal_get_ns_MSP();

  __TZ_set_MSP_NS(ns_msp);

  /* Get the address of non-secure code entry point to jump there */
  unsigned int entry_ptr = tfm_spm_hal_get_ns_entry_point();

  printf("vector_table: %p \n MSP %p \n entry point %p \n ",
         SCB_NS->VTOR, ns_msp, entry_ptr);

  // ns_entry = (nsfptr_t)(*((uint32_t *)((tfm_spm_hal_get_ns_VTOR()) + 4U)));
  // TZ_NONSECURE_FUNC_PTR_DECLARE(ns_entry);
  // ns_entry = TZ_NONSECURE_FUNC_PTR_CREATE(entry_ptr);
  ns_entry = (nsfptr_t)cmse_nsfptr_create(entry_ptr);
}

void core_init(void)
{
  /*Enable fault handler*/
  if (enable_fault_handlers() == TFM_PLAT_ERR_SUCCESS){
    printf("Succsess\n");
  }else{
    printf("Failure\n");
  }

  system_reset_cfg();
  // /* Configures the system reset request properties */
  // if (tfm_spm_hal_system_reset_cfg() == TFM_PLAT_ERR_SUCCESS){
  //   printf("Succsess\n");
  // }else{
  //   printf("Failure\n");
  // }

  __enable_irq();

  /*Setup the for a jump to non-secure code*/
  configure_ns_code();

  if (nvic_interrupt_enable() == TFM_PLAT_ERR_SUCCESS){
    printf("Succsess 2\n");
  }else{
    printf("Failure 2\n");
  }

  TZ_SAU_Enable();
}

/*START Sparrow test*/
void __set_MSPA(uint32_t topOfMainStack) __attribute__((naked));
void __set_MSPA(uint32_t topOfMainStack)
{
  __asm volatile("MSR msp, %0\n\t"
                 "BX lr \n\t"
                 :
                 : "r"(topOfMainStack));
}
static void boot_image(void)
{
  typedef void (*entry_point_t)(void);

  entry_point_t entry_point =
      (entry_point_t)(*(uint32_t *)(0x00050000 + 4U));
  uint32_t stack = *(uint32_t *)(0x00050000);
  *((volatile uint32_t *)(0xE000ED08)) = 0x00050000; // Vector table
  __set_MSPA(stack);

  entry_point();
}
/*END Sparrow test*/
#define cmse_is_nsfptr(p) (!((__INTPTR_TYPE__)(p)&1))


/*---------------------------------------------------------------------------*/
PROCESS(example_s_process, "Example s process");
AUTOSTART_PROCESSES(&example_s_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_s_process, ev, data)
{

  PROCESS_BEGIN();

  printf("SECURE WORLD\n");
  printf("Secure_variable value: %d, memory address: %p \n", secure_variable, &secure_variable);
  leds_single_on(0);


  
  /*Zephyr SPM example*/
  if (SPM_EXAMPLE){
    printf("Running example \n", SPM_EXAMPLE);

    if (enable_fault_handlers() == TFM_PLAT_ERR_SUCCESS){
      printf("Succsess\n");
    }else{
      printf("Failure\n");
    }

    /*Set flash and RAM secure, 
      set non-secure parition non-secure for both flash and RAM
      Set all peripherials non-secure*/

    configuration_Z();

    unsigned int *vtor_ns = (unsigned int)0x50000;

    // vtor_ns[0] = 0x20042840;/*0x200427a0*/;
    printf("NS IMAGE AT 0x%x \n", (unsigned int)vtor_ns);
    printf("NS MSP AT 0x%x \n", vtor_ns[0]);
    printf("NS reset vector AT 0x%x \n", vtor_ns[1]);

    /*Things that isn't necessary but perhaps*/
    /*Configure non-secure stack*/
    tz_nonsecure_setup_conf_t spm_ns_conf = {

        .vtor_ns = (unsigned int)vtor_ns,
        .msp_ns = vtor_ns[0],
        .psp_ns = 0,
        .control_ns.npriv = 0, /* Privileged mode*/
        .control_ns.spsel = 0  /*Use MSP in Thread mode*/

    };

    tz_nonsecure_state_setup(&spm_ns_conf);
    tz_nonsecure_exception_prio_config(1);

    // tz_nbanked_exception_target_state_set(0);
    // tx_nonsecure_system_reset_req_block(IS_ENABLED)

    sau_and_idau_cfg();

    TZ_NONSECURE_FUNC_PTR_DECLARE(reset_ns);
    reset_ns = TZ_NONSECURE_FUNC_PTR_CREATE(vtor_ns[1]);

    if (cmse_is_nsfptr(reset_ns)){
      printf("SPM: prepare to jump to Non-Secure image. \n");
      spu_periph_config_uarte();

      __DSB();
      __ISB();
      reset_ns();
    }else{
      printf("ERROR: WRONG POINTER TYPE\n");
    }
  }

  /*Start of configuration*/
  /* More TF-M like configuration */
  /* Need to use the TF-M linker and startup scripts*/
  if (TFM_EXAMPLE){
    printf("Running example \n", TFM_EXAMPLE);
    
    /* Allow SPU to have precedence over (non-existing) ARMv8-M SAU */
    sau_and_idau_cfg();

    /* Reset everything secure but don't lock anything */
    /* Then configure the non-secure memory addresses in RAM and ROM secure*/
    spu_init_cfg();

    /* Print debug information */
    print_information();
    /*spu_periph_init_cfg() sets all perhiperals to non-secure and allocates GPIOS to non-secure domain*/
    spu_periph_init_cfg();

    /*Enable the fault handlers,
      Enable system reset request for secure world only,
      Setup up the non-secure pointer( non-secure region, MSP, Reset handler)*/
    core_init();

    if (cmse_is_nsfptr(ns_entry)){
      // printf("Jumping to non-secure code \n");
      jump_to_ns_code();
    }else{
      printf("ERROR: WRONG POINTER TYPE\n");
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
