/*
 *	Secure functions, made callable from Non-secure code
 *	This file is to be used by Non-secure code
 *
 *	Needs to be compiled with -cmse flag
 *
 * */
//#if (__ARM_FEATURE_CMSE & 1) == 0
//#error "Need ARMv8-M security extensions"
//#elif (__ARM_FEATURE_CMSE & 2) == 0
//#error "Compile with --cmse in order to make the build secure"
//#endif

//#include <stdio.h>
//#include "stdlib.h"
#include <stdint.h>
#include <arm_cmse.h>
//#include "tz_context.h"
#include "veneer_table.h"


int __attribute__((noclone,cmse_nonsecure_call)) secure_funcX(void){
	return 10;
}
int __attribute__((cmse_nonsecure_entry)) secure_func_entryX(int val){


    return val;
}

int secure_func_non_objX(void){

    return 11;
}
