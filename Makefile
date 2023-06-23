CP=cp -v

RGB133_BIN	:= bin
RGB133_SRC	:= src
RGB133_FW	:= firmware

# TYPE is set to appropriate driver in release build
TYPE=133
ifeq ($(TYPE),133)
	INCLUDE_TYPE=INCLUDE_VISION
	DGC133FW=DGC133FW.BIN
else
    INCLUDE_TYPE=INCLUDE_VISIONLC
    DGC133FW=DGC200FW.BIN
endif

VERBOSE=0
BASE_DIR=""

RGB133_OBJS := \
	$(RGB133_SRC)/rgb133.o \
   $(RGB133_SRC)/rgb133colours.o \
   $(RGB133_SRC)/rgb133drm.o \
	$(RGB133_SRC)/rgb133ioctl.o \
	$(RGB133_SRC)/rgb133irq.o \
   $(RGB133_SRC)/rgb133kernel.o \
   $(RGB133_SRC)/rgb133mapping.o \
	$(RGB133_SRC)/rgb133math.o \
   $(RGB133_SRC)/rgb133peekpoke.o \
   $(RGB133_SRC)/rgb133sg.o \
   $(RGB133_SRC)/rgb133time.o \
   $(RGB133_SRC)/rgb133timer.o \
	$(RGB133_SRC)/rgb133v4l2.o \
	$(RGB133_SRC)/rgb133vidbuf.o \
	$(RGB133_SRC)/rgb133win.o \
   $(RGB133_SRC)/rgb133audio.o \
   $(RGB133_SRC)/rgb133sndcard.o \
   $(RGB133_SRC)/rgb133audiocontrol.o

ifeq ($(RGB133_USER_MODE),YES)
RGB133_OBJS += \
	$(RGB133_SRC)/rgb133registry.o \
	$(RGB133_SRC)/rgb133draw.o \
	$(RGB133_SRC)/rgb133userapi.o \
	$(RGB133_SRC)/rgb133usercapapi.o \
	$(RGB133_SRC)/rgb133userdeviceapi.o \
	$(RGB133_SRC)/rgb133userirqapi.o
endif

OSAPI_OBJS := \
	$(RGB133_SRC)/OSAPI.o

ARCH := $(shell uname -m | sed -e 's/i.86/i386/')
WRAPPER_LIB  := $(RGB133_BIN)/rgb$(TYPE).o
SYS_CORE_LIB := $(RGB133_BIN)/dgc$(TYPE)sys.o

obj-m += rgb$(TYPE).o

ifneq ($(RGB133_USER_MODE),YES)
rgb$(TYPE)-objs := $(RGB133_OBJS) $(OSAPI_OBJS) $(WRAPPER_LIB) $(SYS_CORE_LIB)
else
rgb$(TYPE)-objs := $(RGB133_OBJS)
endif

KERNELDIR      := /lib/modules/$(shell uname -r)
PWD			:= $(shell pwd)

RGB_X_DIR		:= $(obj)
RGB_X_INC		:= -I$(RGB_X_DIR)/include

USER_C_FLAGS := \
        -DKERNEL_MODE=1 -DUSE_1920 -DNEW_APERTURES -DNEW_SCATTER_GATHER_ENTRIES \
        -D$(INCLUDE_TYPE) -DBURST_TRANSFER -DLOG_SG_ENTRIES -DINTERLACE_WORKS -DENABLE_TAGGING=0 \
        -DINCLUDE_OSAPI_EXT -Wno-undef -Wno-unknown-pragmas -Wno-pointer-to-int-cast -Wno-unused-variable \
        -Wno-comment -Wno-char-subscripts -Wno-format -Wno-unused-value \
        -Wno-int-to-pointer-cast -Wno-switch -Wno-strict-prototypes -fno-short-wchar -fno-pie
	
BUILD_CFLAGS := -DRGB133_RELEASE_BUILD

ifeq ($(RGB133_USER_MODE),YES)
        EXTRA_CFLAGS    := $(RGB_X_INC) $(USER_C_FLAGS) $(BUILD_CFLAGS) -DRGB133_USER_MODE -Os $(ARCH_CFLAGS)
else
        EXTRA_CFLAGS    := $(RGB_X_INC) $(USER_C_FLAGS) $(BUILD_CFLAGS) -Os $(ARCH_CFLAGS)
endif

SCRIPTS := $(wildcard ./scripts/*)
ARCHIVES := $(wildcard ./bin/*.o)
CONFIGURATIONS := $(wildcard ./configurations/*)

# Module to build (133 | 200) must be specified when running make
ifeq (,$(filter $(TYPE),133 200))
all:
	$(info When running makefile, specify TYPE=<x>   where <x>={133, 200})
modules: all
modules_install: all
firmware: all
load: all
unload: all

else
all: modules modules_install load

dos2unix:
	$(foreach SCRIPT, $(SCRIPTS),$(shell sed "s/\r//" $(SCRIPT) > $(SCRIPT).tmp))
	$(foreach SCRIPT, $(SCRIPTS),$(shell mv $(SCRIPT).tmp $(SCRIPT)))
	$(foreach SCRIPT, $(SCRIPTS),$(shell chmod +x $(SCRIPT)))
	$(foreach CONFIG, $(CONFIGURATIONS),$(shell chmod 444 $(CONFIG)))

conftest: dos2unix
	./scripts/rgb133config.sh gcc $(ARCH) $(KERNELDIR) $(PWD)/include test_linux_features

modules: conftest
	$(MAKE) -C $(KERNELDIR)/build M=$(PWD) V=$(VERBOSE) modules

modules_install:
	$(MAKE) -C $(KERNELDIR)/build M=$(PWD) modules_install
	$(DEPMOD)

firmware:
	@$(CP) $(RGB_X_DIR)/firmware/$(DGC133FW) /lib/firmware/$(DGC133FW)

load: firmware
	$(DEPMOD)
	@scripts/load_rgb133.sh -f -r

unload:
	@scripts/load_rgb133.sh -f -u

endif
clean:
	@$(foreach ARCHIVE, $(ARCHIVES), mv $(ARCHIVE) $(ARCHIVE).tmp;)
	$(MAKE) -C $(KERNELDIR)/build M=$(PWD) clean
	@$(foreach ARCHIVE, $(ARCHIVES), mv $(ARCHIVE).tmp $(ARCHIVE);)
	
