# Based on https://spin.atomicobject.com/makefile-c-projects/

SRC ?= "./src"
BLD ?= "./build"
DBG ?= "./debug"
BTG ?= "$(BLD)/gibi"
DTG ?= "$(DBG)/gibi"

CC ?= "gcc"

BCFLAGS   ?=
BLDFLAGS  ?=
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
	$(CC) $(BLDOBJ) -o $@ $(LDFLAGS)

$(BLD)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(DCFLAGS) -c $< -o $@

$(DTG): $(DBGOBJ)
	$(CC) $(DBGOBJ) -o $@ $(DLDFLAGS)

$(DBG)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(BCFLAGS) -c $< -o $@

.PHONY: all debug clean

all: $(BTG)

debug: $(DTG)

clean:
	$(RM) -r $(BLD) $(DBG)

-include $(DEPEND)


