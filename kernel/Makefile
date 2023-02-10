include ../compile.mk

KERNEL := ../out/kernel.elf

# c compiler flags particular for the kernel
INTERNALCFLAGS :=            \
	-I./src/                 \
	-Wno-int-to-pointer-cast \

# linker flags particular for the kernel
INTERNALLDFLAGS :=         \
	-Tlinker.ld            \
	-nostdlib              \
	-static

# Use find to glob all *.c files in the directory and extract the object names.
SRCDIR := src
OBJDIR := obj

SRCC = $(call rwildcard,$(SRCDIR),*.c)  
ASMSRC = $(call rwildcard,$(SRCDIR),*.asm)  
OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCC))
OBJS += $(patsubst $(SRCDIR)/%.asm, $(OBJDIR)/%_asm.o, $(ASMSRC))
DIRS = $(wildcard $(SRCDIR)/*)

# Default target.
.PHONY: all
all: $(KERNEL)

src/limine.h:
	curl https://raw.githubusercontent.com/limine-bootloader/limine/trunk/limine.h -o $@

# Link rules for the final kernel executable.
$(KERNEL): $(OBJS) 
	$(LD) $(OBJS) $(LDFLAGS) $(INTERNALLDFLAGS) -o $(KERNEL)

# Compilation rules for *.c files.
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@ mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INTERNALCFLAGS) -c $^ -o $@

# Compilation rules for *.asm files.
$(OBJDIR)/%_asm.o: $(SRCDIR)/%.asm
	@ mkdir -p $(dir $@)
	$(ASM) $(ASMFLAGS) $(INTERNALASMFLAGS) $^ -o $@

# Remove object files and the final executable.
.PHONY: clean
clean:
	rm -rf $(KERNEL) $(OBJS) src/limine.h

.PHONY: setup
setup: src/limine.h