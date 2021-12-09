secure_build = 0

CONTIKI_PROJECT = example_ns #hello-world-test
all: $(CONTIKI_PROJECT)

#$(info    building the normal makefile)
#MODULES += os/services/shell
#MODULES += subdir
#MODULES += os/services/test_module
#MODULES += os/services/simple-energest
#PROJECT_SOURCEFILES += veneer_table.c
CONTIKI = ../../..
include $(CONTIKI)/Makefile.include


#CFLAGS += -mcpu=cortex-m33
#CFLAGS += --specs=nosys.specs
#CFLAGS += -mcmse
#CFLAGS += -mthumb
#CFLAGS += -c

#LDFLAGS += -o example_ns.elf nsclib.o
#LDFLAGS += -o example_ns.elf veneer_table.o
#LDFLAGS += -Xlinker -cmse-implib -out-implib=/build/nrf/nrf5340/dk/application/obj/nsclib.o -T test.ld
