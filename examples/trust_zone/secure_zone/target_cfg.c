/*
 * Copyright (c) 2018-2020 Arm Limited. All rights reserved.
 * Copyright (c) 2020 Nordic Semiconductor ASA.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*HERE*/
#include "defines_config.h"

#include "stdio.h"
#include "target_cfg.h"
#include "region_defs.h"
#include "tfm_includes/tfm_plat_defs.h"
#include "region.h"

#include <spu.h>
#include <nrfx.h>
#include <hal/nrf_gpio.h>

#include "nrf5340_application_bitfields.h"

#define PIN_XL1 0
#define PIN_XL2 1

struct platform_data_t tfm_peripheral_timer0 = {
    NRF_TIMER0_S_BASE,
    NRF_TIMER0_S_BASE + (sizeof(NRF_TIMER_Type) - 1),
};

struct platform_data_t tfm_peripheral_std_uart = {
    NRF_UARTE1_S_BASE,
    NRF_UARTE1_S_BASE + (sizeof(NRF_UARTE_Type) - 1),
};

#ifdef PSA_API_TEST_IPC
struct platform_data_t
    tfm_peripheral_FF_TEST_SERVER_PARTITION_MMIO = {
        FF_TEST_SERVER_PARTITION_MMIO_START,
        FF_TEST_SERVER_PARTITION_MMIO_END};

struct platform_data_t
    tfm_peripheral_FF_TEST_DRIVER_PARTITION_MMIO = {
        FF_TEST_DRIVER_PARTITION_MMIO_START,
        FF_TEST_DRIVER_PARTITION_MMIO_END};

/* This platform implementation uses PSA_TEST_SCRATCH_AREA for
 * storing the state between resets, but the FF_TEST_NVMEM_REGIONS
 * definitons are still needed for tests to compile.
 */
#define FF_TEST_NVMEM_REGION_START 0xFFFFFFFF
#define FF_TEST_NVMEM_REGION_END 0xFFFFFFFF
struct platform_data_t
    tfm_peripheral_FF_TEST_NVMEM_REGION = {
        FF_TEST_NVMEM_REGION_START,
        FF_TEST_NVMEM_REGION_END};
#endif /* PSA_API_TEST_IPC */

/* The section names come from the scatter file */
REGION_DECLARE(Load$$LR$$, LR_NS_PARTITION, $$Base);
REGION_DECLARE(Load$$LR$$, LR_VENEER, $$Base);
REGION_DECLARE(Load$$LR$$, LR_VENEER, $$Limit);
#ifdef BL2
REGION_DECLARE(Load$$LR$$, LR_SECONDARY_PARTITION, $$Base);
#endif /* BL2 */

const struct memory_region_limits memory_regions = {
    .non_secure_code_start =
        (uint32_t)&REGION_NAME(Load$$LR$$, LR_NS_PARTITION, $$Base) /* +
         BL2_HEADER_SIZE*/
    ,

    .non_secure_partition_base =
        (uint32_t)&REGION_NAME(Load$$LR$$, LR_NS_PARTITION, $$Base),

    .non_secure_partition_limit =
        (uint32_t)&REGION_NAME(Load$$LR$$, LR_NS_PARTITION, $$Base) +
        NS_PARTITION_SIZE - 1,

    .veneer_base =
        (uint32_t)&REGION_NAME(Load$$LR$$, LR_VENEER, $$Base),

    .veneer_limit =
        (uint32_t)&REGION_NAME(Load$$LR$$, LR_VENEER, $$Limit),

#ifdef BL2
    .secondary_partition_base =
        (uint32_t)&REGION_NAME(Load$$LR$$, LR_SECONDARY_PARTITION, $$Base),

    .secondary_partition_limit =
        (uint32_t)&REGION_NAME(Load$$LR$$, LR_SECONDARY_PARTITION, $$Base) +
        SECONDARY_PARTITION_SIZE - 1,
#endif /* BL2 */
};

/* To write into AIRCR register, 0x5FA value must be write to the VECTKEY field,
 * otherwise the processor ignores the write.
 */
#define SCB_AIRCR_WRITE_MASK ((0x5FAUL << SCB_AIRCR_VECTKEY_Pos))

enum tfm_plat_err_t enable_fault_handlers(void)
{
    /* Explicitly set secure fault priority to the highest */
    NVIC_SetPriority(SecureFault_IRQn, 0);

    /* Enables BUS, MEM, USG and Secure faults */
    SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk | SCB_SHCSR_BUSFAULTENA_Msk | SCB_SHCSR_MEMFAULTENA_Msk | SCB_SHCSR_SECUREFAULTENA_Msk;
    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t system_reset_cfg(void)
{
    uint32_t reg_value = SCB->AIRCR;

    /* Clear SCB_AIRCR_VECTKEY value */
    reg_value &= ~(uint32_t)(SCB_AIRCR_VECTKEY_Msk);

    /* Enable system reset request only to the secure world */
    reg_value |= (uint32_t)(SCB_AIRCR_WRITE_MASK | SCB_AIRCR_SYSRESETREQS_Msk);

    SCB->AIRCR = reg_value;

    return TFM_PLAT_ERR_SUCCESS;
}

// enum tfm_plat_err_t init_debug(void)
// {
// #if defined(DAUTH_NONE)
//     /* Disable debugging */
//    NRF_CTRLAP->APPROTECT.DISABLE = 0;
//    NRF_CTRLAP->SECUREAPPROTECT.DISABLE = 0;
// #elif defined(DAUTH_NS_ONLY)
//     /* Allow debugging Non-Secure only */
//    NRF_CTRLAP->APPROTECT.DISABLE = NRF_UICR->APPROTECT;
//    NRF_CTRLAP->SECUREAPPROTECT.DISABLE = 0;
// #elif defined(DAUTH_FULL) || defined(DAUTH_CHIP_DEFAULT)
//     /* Allow debugging */
//     /* Use the configuration in UICR. */
//    NRF_CTRLAP->APPROTECT.DISABLE = NRF_UICR->APPROTECT;
//    NRF_CTRLAP->SECUREAPPROTECT.DISABLE = NRF_UICR->SECUREAPPROTECT;
// #else
// #error "No debug authentication setting is provided."
// #endif
//     /* Lock access to APPROTECT, SECUREAPPROTECT */
//    NRF_CTRLAP->APPROTECT.LOCK = CTRLAPPERI_APPROTECT_LOCK_LOCK_Locked << CTRLAPPERI_APPROTECT_LOCK_LOCK_Msk;
//    NRF_CTRLAP->SECUREAPPROTECT.LOCK = CTRLAPPERI_SECUREAPPROTECT_LOCK_LOCK_Locked << CTRLAPPERI_SECUREAPPROTECT_LOCK_LOCK_Msk;

//    return TFM_PLAT_ERR_SUCCESS;
// }

/*----------------- NVIC interrupt target state to NS configuration ----------*/
enum tfm_plat_err_t nvic_interrupt_target_state_cfg(void)
{
    /* Target every interrupt to NS; unimplemented interrupts will be Write-Ignored */
    for (uint8_t i = 0; i < sizeof(NVIC->ITNS) / sizeof(NVIC->ITNS[0]); i++)
    {
        printf(" LOOP : %d \n", i);
        NVIC->ITNS[i] = /*0x50000*/ 0xFFFFFFFF;
    }

    /* Make sure that the SPU is targeted to S state */
    NVIC_ClearTargetState(NRFX_IRQ_NUMBER_GET(NRF_SPU));

#ifdef SECURE_UART1
    /* UARTE1 is a secure peripheral, so its IRQ has to target S state */
    NVIC_ClearTargetState(NRFX_IRQ_NUMBER_GET(NRF_UARTE1));
#endif

    return TFM_PLAT_ERR_SUCCESS;
}

/*----------------- NVIC interrupt enabling for S peripherals ----------------*/
enum tfm_plat_err_t nvic_interrupt_enable(void)
{
    /* SPU interrupt enabling */
    spu_enable_interrupts();

    NVIC_ClearPendingIRQ(NRFX_IRQ_NUMBER_GET(NRF_SPU));
    NVIC_EnableIRQ(NRFX_IRQ_NUMBER_GET(NRF_SPU));

    return TFM_PLAT_ERR_SUCCESS;
}

/*------------------- SAU/IDAU configuration functions -----------------------*/

void sau_and_idau_cfg(void)
{
    /* IDAU (SPU) is always enabled. SAU is non-existent.
     * Allow SPU to have precedence over (non-existing) ARMv8-M SAU.
     */
    TZ_SAU_Disable();
    SAU->CTRL |= SAU_CTRL_ALLNS_Msk;
}

uint32_t tfm_spm_hal_get_ns_VTOR(void)
{
    return memory_regions.non_secure_code_start;
}

uint32_t tfm_spm_hal_get_ns_MSP(void)
{
    return *((uint32_t *)memory_regions.non_secure_code_start);
}

uint32_t tfm_spm_hal_get_ns_entry_point(void)
{
    return *((const uint32_t *)(memory_regions.non_secure_code_start + 4));
}

enum tfm_plat_err_t spu_init_cfg(void)
{
    /*
     * Configure SPU Regions for Non-Secure Code and SRAM (Data)
     * Configure SPU for Peripheral Security
     * Configure Non-Secure Callable Regions
     * Configure Secondary Image Partition for BL2
     */

    /* Explicitly reset Flash and SRAM configuration to all-Secure,
     * in case this has been overwritten by earlier images e.g.
     * bootloader.
     */
    printf("Set everything secure\n");
    spu_regions_reset_all_secure();

    printf("Flash non secure start: %p, end: %p \n", memory_regions.non_secure_partition_base,
           memory_regions.non_secure_partition_limit);
    printf("sram non secure start: %p, end: %p \n", NS_DATA_START,
           NS_DATA_LIMIT);
    printf("nsc non secure start: %p, end: %p \n", memory_regions.veneer_base,
           memory_regions.veneer_limit - 1);
    /* Configures SPU Code and Data regions to be non-secure */

    printf("Set specific things non-secure and lock \n");
    // spu_regions_flash_config_non_secure_id(20,31);
    // spu_regions_sram_config_non_secure_all();

    spu_regions_flash_config_non_secure(memory_regions.non_secure_partition_base,
                                        memory_regions.non_secure_partition_limit);
    spu_regions_sram_config_non_secure(NS_DATA_START, NS_DATA_LIMIT);

    /* Configures veneers region to be non-secure callable */
    printf("Set things non-secure callable\n");
    spu_regions_flash_config_non_secure_callable(memory_regions.veneer_base,
                                                 memory_regions.veneer_limit - 1);

#ifdef BL2
    printf("BL1 is defined \n");
    /* Secondary image partition */
    spu_regions_flash_config_non_secure(memory_regions.secondary_partition_base,
                                        memory_regions.secondary_partition_limit);
#endif /* BL2 */

    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t spu_periph_init_cfg(void)
{
    /* Peripheral configuration */
    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_FPU));
    spu_peripheral_config_non_secure((uint32_t)NRF_FPU, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_REGULATORS));
    spu_peripheral_config_non_secure((uint32_t)NRF_REGULATORS, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_CLOCK));
    spu_peripheral_config_non_secure((uint32_t)NRF_CLOCK, false); /*necesary*/
    // spu_peripheral_config_non_secure((uint32_t)NRF_SPIM0, false);

#ifndef SECURE_UART1
    /* UART1 is a secure peripheral, so we need to leave Serial-Box 1 as Secure */
    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_SPIM1));
    spu_peripheral_config_non_secure((uint32_t)NRF_SPIM1, false);
#endif

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_SPIM4));
    spu_peripheral_config_non_secure((uint32_t)NRF_SPIM4, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_SPIM2));
    spu_peripheral_config_non_secure((uint32_t)NRF_SPIM2, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_SPIM3));
    spu_peripheral_config_non_secure((uint32_t)NRF_SPIM3, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_SAADC));
    spu_peripheral_config_non_secure((uint32_t)NRF_SAADC, false);
    
    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_TIMER0));
    spu_peripheral_config_non_secure((uint32_t)NRF_TIMER0, false);
    
    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_TIMER1));
    spu_peripheral_config_non_secure((uint32_t)NRF_TIMER1, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_TIMER2));
    spu_peripheral_config_non_secure((uint32_t)NRF_TIMER2, false);

    /*Before commeted*/
    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_RTC0));
    spu_peripheral_config_non_secure((uint32_t)NRF_RTC0, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_RTC1));
    spu_peripheral_config_non_secure((uint32_t)NRF_RTC1, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_DPPIC));
    spu_peripheral_config_non_secure((uint32_t)NRF_DPPIC, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_WDT0));
    spu_peripheral_config_non_secure((uint32_t)NRF_WDT0, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_WDT1));
    spu_peripheral_config_non_secure((uint32_t)NRF_WDT1, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_COMP));
    spu_peripheral_config_non_secure((uint32_t)NRF_COMP, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_EGU0));
    spu_peripheral_config_non_secure((uint32_t)NRF_EGU0, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_EGU1));
    spu_peripheral_config_non_secure((uint32_t)NRF_EGU1, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_EGU2));
    spu_peripheral_config_non_secure((uint32_t)NRF_EGU2, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_EGU3));
    spu_peripheral_config_non_secure((uint32_t)NRF_EGU3, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_EGU4));
    spu_peripheral_config_non_secure((uint32_t)NRF_EGU4, false);
#ifndef PSA_API_TEST_IPC
    /* EGU5 is used as a secure peripheral in PSA FF tests */

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_EGU5));
    spu_peripheral_config_non_secure((uint32_t)NRF_EGU5, false);
#endif

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_PWM0));
    spu_peripheral_config_non_secure((uint32_t)NRF_PWM0, false); /*necesary*/

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_PWM1));
    spu_peripheral_config_non_secure((uint32_t)NRF_PWM1, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_PWM2));
    spu_peripheral_config_non_secure((uint32_t)NRF_PWM2, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_PWM3));
    spu_peripheral_config_non_secure((uint32_t)NRF_PWM3, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_PDM0));
    spu_peripheral_config_non_secure((uint32_t)NRF_PDM0, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_I2S0));
    spu_peripheral_config_non_secure((uint32_t)NRF_I2S0, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_IPC));
    spu_peripheral_config_non_secure((uint32_t)NRF_IPC, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_QSPI));
    spu_peripheral_config_non_secure((uint32_t)NRF_QSPI, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_NFCT));
    spu_peripheral_config_non_secure((uint32_t)NRF_NFCT, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_GPIOTE1_NS));
    spu_peripheral_config_non_secure((uint32_t)NRF_GPIOTE1_NS, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_MUTEX));
    spu_peripheral_config_non_secure((uint32_t)NRF_MUTEX, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_QDEC0));
    spu_peripheral_config_non_secure((uint32_t)NRF_QDEC0, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_QDEC1));
    spu_peripheral_config_non_secure((uint32_t)NRF_QDEC1, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_USBD));
    spu_peripheral_config_non_secure((uint32_t)NRF_USBD, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_USBREGULATOR));
    spu_peripheral_config_non_secure((uint32_t)NRF_USBREGULATOR, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_NVMC));
    spu_peripheral_config_non_secure((uint32_t)NRF_NVMC, false);    /*necesary*/

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_P0));
    spu_peripheral_config_non_secure((uint32_t)NRF_P0, false);  /*necesary*/

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_P1));
    spu_peripheral_config_non_secure((uint32_t)NRF_P1, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_VMC));
    spu_peripheral_config_non_secure((uint32_t)NRF_VMC, false);


    /*Here we should have the pheripheral block thing */
    // #define PERIPH(name,reg,config){ name, .id = NRFX_PERIPHERAL_ID_GET(reg), IS_ENABLED(config) }

    // PERIPH("NRF_IPC", NRF_IPC_S, CONFIG_SPM_NRF_IPC_NS);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_IPC_S));
    spu_peripheral_config_non_secure((uint32_t)NRF_IPC_S, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_VMC_S));
    spu_peripheral_config_non_secure((uint32_t)NRF_VMC_S, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_FPU_S));
    spu_peripheral_config_non_secure((uint32_t)NRF_FPU_S, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_EGU1_S));
    spu_peripheral_config_non_secure((uint32_t)NRF_EGU1_S, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_EGU2_S));
    spu_peripheral_config_non_secure((uint32_t)NRF_EGU2_S, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_DPPIC_S));
    spu_peripheral_config_non_secure((uint32_t)NRF_DPPIC_S, false);


    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_GPIOTE1_NS));
    spu_peripheral_config_non_secure((uint32_t)NRF_GPIOTE1_NS, false);

    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_REGULATORS_S));
    spu_peripheral_config_non_secure((uint32_t)NRF_REGULATORS_S, false);


    /* DPPI channel configuration */
    spu_dppi_config_non_secure(false);



    /* GPIO pin configuration (P0 and P1 ports) */
    spu_gpio_config_non_secure(0, false);
    spu_gpio_config_non_secure(1, false);

    /* Configure properly the XL1 and XL2 pins so that the low-frequency crystal
     * oscillator (LFXO) can be used.
     * This configuration can be done only from secure code, as otherwise those
     * register fields are not accessible.  That's why it is placed here.
     */
    nrf_gpio_pin_mcu_select(PIN_XL1, NRF_GPIO_PIN_MCUSEL_PERIPHERAL);
    nrf_gpio_pin_mcu_select(PIN_XL2, NRF_GPIO_PIN_MCUSEL_PERIPHERAL);

    /* Enable the instruction and data cache (this can be done only from secure
     * code; that's why it is placed here).
     */
    NRF_CACHE->ENABLE = CACHE_ENABLE_ENABLE_Enabled;

    return TFM_PLAT_ERR_SUCCESS;
}

void spu_periph_configure_to_secure(uint32_t periph_num)
{
    spu_peripheral_config_secure(periph_num, true);
}

void spu_periph_configure_to_non_secure(uint32_t periph_num)
{
    spu_peripheral_config_non_secure(periph_num, true);
}


void spu_periph_config_uarte(void)
{
    NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_UARTE0));
    spu_peripheral_config_non_secure((uint32_t)NRF_UARTE0, false);

    // irq_target_state_set(NRFX_IRQ_NUMBER_GET(NRF_UARTE0),0);
}

void configuration_Z(void)
{

    
    zephyr_config_test();
    spu_periph_init_cfg();    

    /*peripherials configurations*/
    // NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_P0));
    // spu_peripheral_config_non_secure((uint32_t)NRF_P0, false);

    // NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_CLOCK));
    // spu_peripheral_config_non_secure((uint32_t)NRF_CLOCK, false);

    // NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_RTC0));
    // spu_peripheral_config_non_secure((uint32_t)NRF_RTC0, false);

    // NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_RTC1));
    // spu_peripheral_config_non_secure((uint32_t)NRF_RTC1, false);

    // NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_NVMC));
    // spu_peripheral_config_non_secure((uint32_t)NRF_NVMC, false);

    // NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_UARTE1));
    // spu_peripheral_config_non_secure((uint32_t)NRF_UARTE1, false);

    // NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_UARTE2));
    // spu_peripheral_config_non_secure((uint32_t)NRF_UARTE2, false);

    // NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_TWIM2));
    // spu_peripheral_config_non_secure((uint32_t)NRF_TWIM2, false);

    // NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_SPIM3));
    // spu_peripheral_config_non_secure((uint32_t)NRF_SPIM3, false);

    // NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_TIMER0));
    // spu_peripheral_config_non_secure((uint32_t)NRF_TIMER0, false);

    // NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_TIMER1));
    // spu_peripheral_config_non_secure((uint32_t)NRF_TIMER1, false);

    // NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_TIMER2));
    // spu_peripheral_config_non_secure((uint32_t)NRF_TIMER2, false);

    // NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_SAADC));
    // spu_peripheral_config_non_secure((uint32_t)NRF_SAADC, false);

    // NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_PWM1));
    // spu_peripheral_config_non_secure((uint32_t)NRF_PWM1, false);

    // NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_PWM2));
    // spu_peripheral_config_non_secure((uint32_t)NRF_PWM2, false);

    // NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_PWM3));
    // spu_peripheral_config_non_secure((uint32_t)NRF_PWM3, false);

    // // NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_WDT));
    // // spu_peripheral_config_non_secure((uint32_t)NRF_WDT, false);

    // /* ~ ~ ~ ~ */
    // NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_IPC));
    // spu_peripheral_config_non_secure((uint32_t)NRF_IPC, false);

    // NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_VMC));
    // spu_peripheral_config_non_secure((uint32_t)NRF_VMC, false);

    // NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_FPU));
    // spu_peripheral_config_non_secure((uint32_t)NRF_FPU, false);

    // NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_EGU1));
    // spu_peripheral_config_non_secure((uint32_t)NRF_EGU1, false);

    // NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_EGU2));
    // spu_peripheral_config_non_secure((uint32_t)NRF_EGU2, false);

    // NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_DPPIC));
    // spu_peripheral_config_non_secure((uint32_t)NRF_DPPIC, false);

    // /*Little bit different*/
    // NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_GPIOTE1_NS));
    // spu_peripheral_config_non_secure((uint32_t)NRF_GPIOTE1_NS, false);

    // NVIC_DisableIRQ(NRFX_IRQ_NUMBER_GET(NRF_REGULATORS));
    // spu_peripheral_config_non_secure((uint32_t)NRF_REGULATORS, false);


    // /* ~ ~ ~ ~ ~ */
    //     /* DPPI channel configuration */
    // spu_dppi_config_non_secure(false);



    // /* GPIO pin configuration (P0 and P1 ports) */
    // spu_gpio_config_non_secure(0, false);
    // spu_gpio_config_non_secure(1, false);

    // /* Configure properly the XL1 and XL2 pins so that the low-frequency crystal
    //  * oscillator (LFXO) can be used.
    //  * This configuration can be done only from secure code, as otherwise those
    //  * register fields are not accessible.  That's why it is placed here.
    //  */
    //  nrf_gpio_pin_mcu_select(PIN_XL1, NRF_GPIO_PIN_MCUSEL_PERIPHERAL);
    //  nrf_gpio_pin_mcu_select(PIN_XL2, NRF_GPIO_PIN_MCUSEL_PERIPHERAL);

    // /* Enable the instruction and data cache (this can be done only from secure
    //  * code; that's why it is placed here).
    //  */

    //  NRF_CACHE->ENABLE = CACHE_ENABLE_ENABLE_Enabled;
}