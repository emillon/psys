# Copyright (C) 2001-2003 by Simon Nieuviarts

# Files to compile
FILES=$(wildcard *.S *.c) printf.c sprintf.c doprnt.c panic.c string.c
DIRS=. ../shared

# crt0.o must be the first object linked
OBJS=$(strip crt0.o $(filter-out crt0.o,$(notdir $(patsubst %.S,%.o,$(patsubst %.c,%.o,$(FILES))))))

CC=gcc
AS=gcc
LD=ld

KFLAGS=-Wall -Werror -g -gstabs -pipe -std=c99 -nostdinc
#KFLAGS=-Wall -O2 -fomit-frame-pointer -pipe -std=c99 -nostdinc

DEF=
#DEF=-DTELECOM_TST

INC=$(patsubst %,-I%,$(DIRS))
CCL=$(CC) -Werror $(KFLAGS) $(DEF) $(INC)
ASL=$(AS) -DASSEMBLER $(DEF) $(INC)
DEPS=$(patsubst %.o,%.d,$(OBJS))

# Where to search for source files
vpath %.c $(DIRS)
vpath %.h $(DIRS)
vpath %.S $(DIRS)

# Targets that are not files
.PHONY: clean all

# The default target
all: prog.bin

# Automatic rules to build dependency files
%.d: %.c
	@$(SHELL) -ec '$(CCL) -M $< | sed '\''s|\($*\)\.o[ :]*|\1.o $@ : Makefile |g'\'' > $@; [ -s $@ ] || rm -f $@'

%.d: %.S
	@$(SHELL) -ec '$(ASL) -M $< | sed '\''s|\($*\)\.o[ :]*|\1.o $@ : Makefile |g'\'' > $@; [ -s $@ ] || rm -f $@'

# Build and include dependency files
-include $(DEPS)

# Automatic rules to build the object files
%.o: %.S
	$(ASL) -c $< -o $@

%.o: %.c
	$(CCL) -c $< -o $@

%.s: %.c
	$(CCL) -S -c $< -o $@

prog.debug: prog.lds $(OBJS)
	$(LD) -e entry -Tprog.lds $(OBJS) -o $@

prog.bin: prog.debug
	objcopy -O binary prog.debug prog.bin

clean:
	rm -f $(OBJS) $(DEPS) prog.bin prog.debug
