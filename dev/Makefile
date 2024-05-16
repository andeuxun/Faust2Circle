#
# Makefile
#

#USE_VCHIQ_SOUND = 1

CIRCLEHOME = ../..

OBJS	= main.o kernel.o circle.o

ifeq ($(strip $(USE_VCHIQ_SOUND)),1)
LIBS	= $(CIRCLEHOME)/addon/vc4/sound/libvchiqsound.a \
	  $(CIRCLEHOME)/addon/vc4/vchiq/libvchiq.a \
	  $(CIRCLEHOME)/addon/linux/liblinuxemu.a

CFLAGS += -DUSE_VCHIQ_SOUND=$(USE_VCHIQ_SOUND)
endif

LIBS	+= $(CIRCLEHOME)/lib/sched/libsched.a \
	   $(CIRCLEHOME)/lib/sound/libsound.a \
	   $(CIRCLEHOME)/lib/usb/libusb.a \
	   $(CIRCLEHOME)/lib/input/libinput.a \
	   $(CIRCLEHOME)/lib/fs/libfs.a \
	   $(CIRCLEHOME)/lib/libcircle.a

LIBS	+= math.h \
	   /usr/local/include/faust/dsp/fastmath.cpp

# Lib from the expanded version of the dsp, might be toot much
LIBS	+= /usr/local/share/faust/stdfaust.lib \
	   /usr/local/share/faust/oscillators.lib \
	   /usr/local/share/faust/maths.lib \
	   /usr/local/share/faust/platform.lib

include ../Rules.mk

-include $(DEPS)
