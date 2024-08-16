# Based on https://spin.atomicobject.com/makefile-c-projects/

SRC ?= "./src"
BLD ?= "./build"
TGT ?= "$(BLD)/gibi"

CC  ?= "gcc"

CFLAGS   ?= 
CPPFLAGS ?=
LDFLAGS  ?=

SOURCE := $(shell find $(SRC) -name "*.c")
OBJECT := $(SOURCE:%=$(BLD)/%.o)
DEPEND := $(OBJECT:.o=.d)

DEP_INCL := $(addprefix -I,$(shell find $(SRC) -type d))
CPPFLAGS += $(DEP_INCL) "-MMD" "-MP"

$(TGT): $(OBJECT)
	$(CC) $(OBJECT) -o $@ $(LDFLAGS)

$(BLD)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: all clean

all: $(TGT)

clean:
	$(RM) -r $(BLD)

-include $(DEPEND)


