/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 0x40000 /*0x100000*/
  /*EXTFLASH (rx) : ORIGIN = 0x10000000, LENGTH = 0x8000000*/
  RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 0x30000
  /*RAM (rwx) : ORIGIN = 0x20040000, LENGTH = 0x40000*/
  /*NON_SECURE_FUNC (rx) : ORIGIN = 0x00050000, LENGTH = 0x00000100*/ 

}
/*MEMORY
{
    FLASH (rx) : ORIGIN = 0x0, LENGTH = 0x8000
    SRAM (wx) : ORIGIN = 0x20000000, LENGTH = (64 * 1K)
    IDT_LIST (wx) : ORIGIN = (0x20000000 + (64 *1K)), LENGTH = 2K
}*/


INCLUDE "nrf_common.ld"
/*INCLUDE "nrf_common_original.ld"*/