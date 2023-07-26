include ../../compile.mk

LIBNAME = $(notdir $(shell pwd))

# default output path of the library
LIB := bin/$(LIBNAME).a

SRCC = $(call rwildcard,$(SRCDIR),*.c)  
ASMSRC = $(call rwildcard,$(SRCDIR),*.asm)  
OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCC))
OBJS += $(patsubst $(SRCDIR)/%.asm, $(OBJDIR)/%_asm.o, $(ASMSRC))
DIRS = $(wildcard $(SRCDIR)/*)
CFLAGS += -mcmodel=large

define parse_dependencies
$(foreach lib, $(1), \
	$(eval CFLAGS+=-I../$(lib)/inc ) \
)
endef

# Default target.
.PHONY: all
all: $(LIB)

# Link rules for the final libc
$(LIB): $(OBJS)
	mkdir -p bin
	$(AR) -crs $@ $(OBJS)

# Compilation rules for *.c files.
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@ mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $^ -o $@

# Compilation rules for *.asm files.
$(OBJDIR)/%_asm.o: $(SRCDIR)/%.asm
	@ mkdir -p $(dir $@)
	$(ASM) $(ASMFLAGS) $^ -o $@

clean:
	rm -rf $(LIB)