secure_build = 1

CONTIKI_PROJECT = example_s
all: $(CONTIKI_PROJECT)

PROJECT_SOURCEFILES += spu.c
PROJECT_SOURCEFILES += target_cfg.c
#PROJECT_SOURCEFILES += test.c

CONTIKI = ../../..
include $(CONTIKI)/Makefile.include

CFLAGS += -mcpu=cortex-m33
CFLAGS += --specs=nosys.specs


CFLAGS += -mcmse
CFLAGS += -mthumb

#For DBG
CFLAGS += -O0 -ggdb2 -g2#-O0 -g3 -lm -Wall #-g
LDFLAGS += -O0 -ggdb2 -g2# -O0 -g3 -lm -Wall #-g
