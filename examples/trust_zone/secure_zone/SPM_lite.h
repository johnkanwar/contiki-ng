/*
 *	Secure functions, made callable from Non-secure code 
 *	This file is to be used by Non-secure code
 *
 *
 *
 * */

//#include <stdio.h>
//#include "stdlib.h"
#include "nrf5340_application.h" //Includes core_cm33 
#include "system_nrf5340_application.h" 
#include "arm_cmse.h"


/* A convenient struct to include all required Non-Secure
 * state configuration.
 */

typedef struct tz_nonsecure_setup_conf {
	uint32_t msp_ns;
	uint32_t psp_ns;
	uint32_t vtor_ns;
	struct {
		uint32_t npriv:1;
		uint32_t spsel:1;
		uint32_t reserved:30;
	} control_ns;
} tz_nonsecure_setup_conf_t;

void configure_nonsecure_vtor_offset(uint32_t vtor_ns);
void tz_nonsecure_exception_prio_config(int secure_boost);
void tz_nbanked_exception_target_state_set(int secure_state);
void tz_nonsecure_system_reset_req_block(int block);
void tz_sau_configure(int enable, int allns);
void spm_configure_ns(const tz_nonsecure_setup_conf_t *spm_ns_conf);
uint32_t tz_sau_number_of_regions_get(void);

//void spm_jump(void);
