PROJECT = MIDIControl-V2
BUILD_DIR = bin

SHARED_DIR = ../libraries
CFILES = main.c
CFILES += setup.c
CFILES += led.c
CFILES += usb.c
CFILES += oled.c
CFILES += oled_fonts.c


# TODO - you will need to edit these two lines!
DEVICE=stm32g431cbt

# You shouldn't have to edit anything below here.
VPATH += $(SHARED_DIR)
INCLUDES += $(patsubst %,-I%, . $(SHARED_DIR))
OPENCM3_DIR=../libopencm3

include $(OPENCM3_DIR)/mk/genlink-config.mk
include ../rules.mk
include $(OPENCM3_DIR)/mk/genlink-rules.mk
