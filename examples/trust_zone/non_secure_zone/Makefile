CONTIKI_PROJECT = example_ns #hello-world-test
all: $(CONTIKI_PROJECT)

#$(info    building the normal makefile)
#MODULES += subdir
#MODULES += os/services/test_module
#MODULES += os/services/simple-energest
#PROJECT_SOURCEFILES += veneer_table.c
CONTIKI = ../../..
include $(CONTIKI)/Makefile.include

LDFLAGS += -o example_ns.elf nsclib.o
#LDFLAGS += -Xlinker -cmse-implib -out-implib=/build/nrf/nrf5340/dk/application/obj/nsclib.o -T test.ld


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
# Base address 0x50003000
