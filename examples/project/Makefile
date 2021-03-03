CONTIKI_PROJECT = easy_jammer
all: $(CONTIKI_PROJECT)

PROJECT_SOURCEFILES += ble_dtm.c
PROJECT_SOURCEFILES += ble_dtm_hw_nrf52.c
CONTIKI = ../..
include $(CONTIKI)/Makefile.include
CFLAGS+=-I$(CONTIKI)
