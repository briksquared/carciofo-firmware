TARGET = cloudseed
CPP_SOURCES = cloudseed.cpp
C_INCLUDES += -I../../../loewenzahnhonig-firmware/src/cloudseed
LIBDAISY_DIR = ../../lib/libDaisy
DAISYSP_DIR = ../../lib/DaisySP
CLOUDSEED_DAISY_DIR = ../../../loewenzahnhonig-firmware/lib/cloudseed-daisy
CLOUDSEED_BOOST = 0
OPT = -Os
include $(CLOUDSEED_DAISY_DIR)/cloudseed.mk
$(BUILD_DIR)/cloudseed.o: OPT = -O2
