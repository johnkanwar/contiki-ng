CONTIKI_PROJECT = example_s
all: $(CONTIKI_PROJECT)

MODULES += os/services/shell
#MODULES += arch/cpu/arm/CMSIS
#PROJECT_SOURCEFILES += veneer_table.c
#PROJECT_SOURCEFILES += spm_uart.c
#PROJECT_SOURCEFILES += spm.c
PROJECT_SOURCEFILES += spu.c
PROJECT_SOURCEFILES += target_cfg.c
#PROJECT_SOURCEFILES += boot_hal.c
#PROJECT_SOURCEFILES += spm_hal.c

CONTIKI = ../../..
include $(CONTIKI)/Makefile.include

#$(CC) $(DEFINES) -E -P -o output.lds -x c-header nordic_nrf_s.ld
#CFLAGS +=-DNRF5340_XXAA_APPLICATION=1

CFLAGS += -mcpu=cortex-m33
CFLAGS += --specs=nosys.specs


CFLAGS += -mcmse
CFLAGS += -mthumb
CLAGSS += -c
#CLAGSS += -Ofast
#LDFLAGS += -o example_ns.elf nsclib.o
#LDFLAGS += -o example_ns.elf veneer_table.o
#LDFLAGS += nsclib.o -o example_ns.elf

#LDFLAGS +=  -o -nsc_test.o -mcpu=cortex-m33 -specs=nosys.specs -mcmse -mthumb -c veneer_table.c
#LDFLAGS += -Xlinker -cmse-implib -Xlinker -out-implib=./nsclib.o -Xlinker -T test.ld # works
#Secure project needs to be built first.

#The small redirect functions in the NSC region are also called „veneer“
#functions or „SG stups“, because they are so small doing only one SG
#instruction and a branch. The linker file of the secure firmware
#must place any veneer code in the secure memory region which the
#secure firmware will declare as NSC as part of its secure boot
#process before jumping in the non-secure firmware.