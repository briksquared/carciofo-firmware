# Carciofo Nimbus — Clouds DSP from the Löwenzahnhonig port, Carciofo I/O.
TARGET = nimbus

NIMBUS = ../../../loewenzahnhonig-firmware/src/nimbus

CPP_SOURCES = nimbus.cpp \
	$(NIMBUS)/dsp/correlator.cpp \
	$(NIMBUS)/dsp/granular_processor.cpp \
	$(NIMBUS)/dsp/pvoc/frame_transformation.cpp \
	$(NIMBUS)/dsp/pvoc/phase_vocoder.cpp \
	$(NIMBUS)/dsp/pvoc/stft.cpp \
	$(NIMBUS)/resources.cpp

C_INCLUDES += -I$(NIMBUS) -I$(NIMBUS)/dsp -I$(NIMBUS)/dsp/pvoc -I$(NIMBUS)/dsp/fx

# -O2 overflows the Seed's 128 KB internal flash by ~900 B.
OPT = -Os

LIBDAISY_DIR = ../../lib/libDaisy
DAISYSP_DIR = ../../lib/DaisySP
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
