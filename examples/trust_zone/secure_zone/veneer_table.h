/*
 *	Secure functions, made callable from Non-secure code
 *	This file is to be used by Non-secure code
 *
 *
 *
 * */
#ifndef VENEER_TABLE_H
#define VENEER_TABLE_H



//#include <stdio.h>
//#include "stdlib.h"
//#include <arm_cmse.h>

typedef void (*ns_funcptr) (void) __attribute__((cmse_nonsecure_call));

void secure_funcX(ns_funcptr callback);
int secure_func_entryX(int val);
int secure_func_non_objX(void);

#endif /*VENEER_TABLE_H*/
