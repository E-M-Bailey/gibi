# Based on https://spin.atomicobject.com/makefile-c-projects/

.DEFAULT_GOAL := all

SRC ?= "./src"
BLD ?= "./build"
DBG ?= "./debug"
BTG ?= "$(BLD)/gibi"
DTG ?= "$(DBG)/gibi"

CC ?= "gcc"

CFLAGS   ?= -O2
LDFLAGS  ?=
BCFLAGS  ?=
BLDFLAGS ?=
DCFLAGS  ?= -g
DLDFLAGS ?=

SOURCE := $(shell find $(SRC) -name "*.c")
BLDOBJ := $(SOURCE:%=$(BLD)/%.o)
BLDDEP := $(BLDOBJ:.o=.d)
DBGOBJ := $(SOURCE:%=$(DBG)/%.o)
DBGDEP := $(DBGOBJ:.o=.d)

DEP_INCL := $(addprefix -I,$(shell find $(SRC) -type d))
CPPFLAGS += $(DEP_INCL) "-MMD" "-MP"

$(BTG): $(BLDOBJ)
	$(CC) $(BLDOBJ) -o $@ $(LDFLAGS) $(BLDFLAGS)

$(BLD)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DCFLAGS) -c $< -o $@

$(DTG): $(DBGOBJ)
	$(CC) $(DBGOBJ) -o $@ $(LDFLAGS) $(DLDFLAGS)

$(DBG)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(BCFLAGS) -c $< -o $@

.PHONY: all gibi debug clean

all: gibi debug

gibi: $(BTG)

debug: $(DTG)

clean:
	$(RM) -r $(BLD) $(DBG)

-include $(DEPEND)

