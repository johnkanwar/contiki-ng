
/* Memories definition */
MEMORY
{
        ROM_NSC   (rx)    : ORIGIN = 0x20000,  LENGTH = 4K    /* Non-Secure Call-able region */
}

SECTIONS{

   .gnu.sgstubs :
   {
       . = ALIGN(8);
       *(.gnu.sgstubs*)   /* Secure Gateway stubs */
       . = ALIGN(8);
   } >ROM_NSC           /*FLASHEN*/

}

/*The memory address is chosen by Zephyrs Konfig file*/

/*0x5000 0000*/
/*0xc744*/

/*0x000000000000aae4*/
/*0x0000000000503500*/
/*0x000000000000c744*/
/*0x0000000020000394*/
/*0x20000*/
/*0x50003000*/
/*0x41000000*/
