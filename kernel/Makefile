include ../compile.mk

OBJCOPY := ~/cross_compiler/bin/x86_64-elf-objcopy

KERNEL := ../out/kernel.elf
FONT := ../kfont.psf

# c compiler flags particular for the kernel
INTERNALCFLAGS :=              \
	-I./src/                   \
	-Wno-int-to-pointer-cast   \
	-fstack-protector    	   \

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

vendor:
	-curl https://raw.githubusercontent.com/limine-bootloader/limine/trunk/limine.h -o src/limine.h

# Link rules for the final kernel executable.
$(KERNEL): $(OBJS)
	$(OBJCOPY) -O elf64-x86-64 -B i386 -I binary $(FONT) $(OBJDIR)/kfont.o
	$(LD) $(OBJS) $(OBJDIR)/kfont.o $(LDFLAGS) $(INTERNALLDFLAGS) -o $(KERNEL)

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
	rm -rf $(KERNEL) $(OBJS)

.PHONY: distclean
distclean: clean
	rm -rf src/limine.h

.PHONY: setup
setup: vendor
