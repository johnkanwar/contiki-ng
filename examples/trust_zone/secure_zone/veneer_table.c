/*
 *	Secure functions, made callable from Non-secure code
 *	This file is to be used by Non-secure code
 *
 *	Needs to be compiled with -cmse flag
 *
 * */
#if (__ARM_FEATURE_CMSE & 1) == 0
#error "Need ARMv8-M security extensions"
#elif (__ARM_FEATURE_CMSE & 2) == 0
#error "Compile with --mcmse in order to make the build secure"
#endif

#include <stdio.h>
#include <stdint.h>
#include <arm_cmse.h>
//#include "tz_context.h"
#include "veneer_table.h"

/*
    The other direction, meaning the secure firmware calling
    a non-secure function, works out of the box, if the non-secure
    function address is somehow given to secure code. Normally this
    is done by defining secure gateway functions with a non-secure
    callback pointer as parameter:
*/
void __attribute__((cmse_nonsecure_call, noclone)) secure_funcX(ns_funcptr callback){
    ns_funcptr cb = callback; // save volatile pointer from non-secure code

    // check if given pointer to non-secure memory is actually non-secure as expected
    cb = cmse_check_address_range(cb, sizeof(cb), CMSE_NONSECURE);

    if (cb != 0){
        printf("Pointer is correct \n");
        cb(); // invoke non-secure call back function
    }
    else{
        printf("Pointer is incorrect \n");
    }
    printf("End of func \n");
}
int __attribute__((cmse_nonsecure_entry, noclone)) secure_func_entryX(int val){

    return val;
}

int secure_func_non_objX(void){

    return 11;
}
