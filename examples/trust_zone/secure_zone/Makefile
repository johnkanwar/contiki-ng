CONTIKI_PROJECT = example_s
all: $(CONTIKI_PROJECT)

PROJECT_SOURCEFILES += veneer_table.c
#PROJECT_SOURCEFILES += secure_world/secure_world.c
CONTIKI = ../../..
include $(CONTIKI)/Makefile.include

CFLAGS += -mcpu=cortex-m33
CFLAGS += --specs=nosys.specs


CFLAGS += -mcmse
CFLAGS += -mthumb
CLAGSS += -c

#LDFLAGS +=  -o -nsc_test.o -mcpu=cortex-m33 -specs=nosys.specs -mcmse -mthumb -c veneer_table.c
LDFLAGS += -Xlinker -cmse-implib -Xlinker -out-implib=./nsclib.o -Xlinker -T test.ld # works
#LDFLAGS += -Xlinker -cmse-implib -Xlinker -out-implib=./nsclib.o -Xlinker -T test.ld # works
#Secure project needs to be built first.

#The small redirect functions in the NSC region are also called „veneer“
#functions or „SG stups“, because they are so small doing only one SG
#instruction and a branch. The linker file of the secure firmware
#must place any veneer code in the secure memory region which the
#secure firmware will declare as NSC as part of its secure boot
#process before jumping in the non-secure firmware.


# place any venner code in the secure memory region
# which the secure frimware will declare as NSC as part of its
# secure boot process before jumping in the non-secure firmware.
# Base address 0x50003000 but not really ?
