/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys )

MEMORY
{

  FLASH (rx) : ORIGIN = 0x00050000, LENGTH = 0x30000 /*0x100000*/
  EXTFLASH (rx) : ORIGIN = 0x10000000, LENGTH = 0x8000000
  RAM (rwx) : ORIGIN = 0x20040000, LENGTH = 0x40000
  /*RAM (rwx) : ORIGIN = 0x20040000, LENGTH = 0x40000*/
  /*NON_SECURE_FUNC (rx) : ORIGIN = 0x00050000, LENGTH = 0x00000100*/ 





}


INCLUDE "nrf_common_ns_my.ld"
/*INCLUDE "nrf_common_original.ld"*/