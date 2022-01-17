secure_build = 0

CONTIKI_PROJECT = example_ns
all: $(CONTIKI_PROJECT)

CONTIKI = ../../..
include $(CONTIKI)/Makefile.include

LDFLAGS += -o example_ns.elf #test.o

#For secure or non-secure
CFLAGS += -D NRF_TRUSTZONE_NONSECURE=1
#For debug reasons
CFLAGS += -O0 -ggdb2 -g2#-O0 -g3 -lm -Wall #-g
LDFLAGS += -O0 -ggdb2 -g2# -O0 -g3 -lm -Wall #-g
