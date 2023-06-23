BIN=./bin
DEPS=./bin
LIBS=./lib

CC			:= gcc
LINK		:= $(CC) 
SHARED	:= $(CC) -shared
STATIC	:= $(CC) -static
RM			:= rm 

SRC		:= ./src

ENUM_TARGET_NAME		:= Enumerate
ENUM_TARGET				:= $(BIN)/$(ENUM_TARGET_NAME)
CAPTURE_TARGET_NAME	    := Capture
CAPTURE_TARGET			:= $(BIN)/$(CAPTURE_TARGET_NAME)
CAPTURE_AUDIO_TARGET_NAME	    := CaptureAudio
CAPTURE_AUDIO_TARGET			:= $(BIN)/$(CAPTURE_AUDIO_TARGET_NAME)
DISCO_TARGET_NAME       := Discovery
DISCO_TARGET            := $(BIN)/$(DISCO_TARGET_NAME)
GETFMT_TARGET_NAME       := GetFmt
GETFMT_TARGET            := $(BIN)/$(GETFMT_TARGET_NAME)

ENUM_OBJS	:= \
	$(BIN)/Enumerate.o
	
CAPTURE_OBJS	:= \
	$(BIN)/Capture.o \
	$(BIN)/CaptureThread.o \
	$(BIN)/CaptureCmdLine.o \
	$(BIN)/CaptureMain.o

CAPTURE_AUDIO_OBJS   := \
   $(BIN)/CaptureAudio.o	

DISCO_OBJS      := \
	$(BIN)/discovery.o

GETFMT_OBJS      := \
	$(BIN)/GetFormat.o

ENUM_LIBS	:= \

CAPTURE_LIBS	:= \
	-lpthread
	
CAPTURE_AUDIO_LIBS   := \
   -lm -lasound -lpthread
   
INCLUDES	:= \
	-I./include \
	-I../../include

ifeq ($(DEBUG),YES)
DBGFLAGS	:= -g -DCAPDEBUG
endif

TARGET_LIBS	:= 
	
vpath %.c $(SRC)

TARGET := $(ENUM_TARGET) $(CAPTURE_TARGET) $(CAPTURE_AUDIO_TARGET) $(DISCO_TARGET) $(GETFMT_TARGET)

#.PHONEY: service run

all: $(TARGET)

.PHONY: Enum Capture CaptureAudio Disco GetFmt

-include $(ENUM_OBJS:.o=.d)
-include $(CAPTURE_OBJS:.o=.d)
-include $(CAPTURE_AUDIO_OBJS:.o=.d)
-include $(DISCO_OBJS:.o=.d)
-include $(GETFMT_OBJS:.o=.d)

Enum: $(ENUM_TARGET)

$(ENUM_TARGET): $(ENUM_OBJS)
	$(LINK) -o $@ $(ENUM_OBJS) $(ENUM_LIBS)

Capture: $(CAPTURE_TARGET)

$(CAPTURE_TARGET): $(CAPTURE_OBJS)
	$(LINK) -o $@ $(CAPTURE_OBJS) $(CAPTURE_LIBS)
	
CaptureAudio: $(CAPTURE_AUDIO_TARGET)

$(CAPTURE_AUDIO_TARGET): $(CAPTURE_AUDIO_OBJS)
	$(LINK) -o $@ $(CAPTURE_AUDIO_OBJS) $(CAPTURE_AUDIO_LIBS)

Disco: $(DISCO_TARGET)

$(DISCO_TARGET): $(DISCO_OBJS)
	$(LINK) -o $@ $(DISCO_OBJS)

GetFmt: $(GETFMT_TARGET)

$(GETFMT_TARGET): $(GETFMT_OBJS)
	$(LINK) -o $@ $(GETFMT_OBJS)

$(BIN)/%.o:	%.c Makefile
	$(CC) $(DBGFLAGS) $(INCLUDES) -o $@ -c $<
	
$(DEPS)/%.d: %.c
	@$(CC) -c -MM $(DBGFLAGS) $(INCLUDES) $^ > $(DEPS)/$*.d
	@mv -f $(DEPS)/$*.d $*.d.tmp
	@sed -e 's|.*:|$(DEPS)/$*.o:|' < $*.d.tmp > $(DEPS)/$*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	sed -e 's/^ *//' -e 's/$$/:/' >> $(DEPS)/$*.d
	@rm -f $*.d.tmp

clean:
	find $(BIN) -iname "*.o" | xargs -i $(RM) {}	
	find $(BIN) -iname "*.d" | xargs -i $(RM) {}	
	find $(BIN) -iname "*.so" | xargs -i $(RM) {}
	find $(BIN) -iname "$(ENUM_TARGET_NAME)" | xargs -i $(RM) {}
	find $(BIN) -iname "$(CAPTURE_TARGET_NAME)" | xargs -i $(RM) {}
	find $(BIN) -iname "$(CAPTURE_AUDIO_TARGET_NAME)" | xargs -i $(RM) {}
	find $(BIN) -iname "$(DISCO_TARGET_NAME)" | xargs -i $(RM) {}
	find $(BIN) -iname "$(GETFMT_TARGET_NAME)" | xargs -i $(RM) {}
