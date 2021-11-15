/*
 * Secure example.
 */

#include "contiki.h"
#include <stdio.h> /* For printf() */

#include <arm_cmse.h> // CMSE definitions
// #include "veneer_table.h"
#include "target_cfg.h"

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
  /*Comparing the position of Reset Handler does it seem like
    it returns an address off by one, that is the reason why
    it is - 1U. Have tested both without and with -1U*/
  unsigned int entry_ptr = tfm_spm_hal_get_ns_entry_point() /*- 1U*/;


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

/*---------------------------------------------------------------------------*/
PROCESS(example_s_process, "Example s process");
AUTOSTART_PROCESSES(&example_s_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_s_process, ev, data)
{

  PROCESS_BEGIN();

  printf("SECURE WORLD\n");
  printf("Secure_variable value: %d, memory address: %p \n", secure_variable, &secure_variable);

  core_init();

  /*Start of configuration*/
  sau_and_idau_cfg();

  // printf("Before boot_image() \n");
  //  boot_image();

  __DSB();
  __ISB();

  printf("Jumping to non-secure code \n");
  jump_to_ns_code();
  printf("This shouldn't print? \n");
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
