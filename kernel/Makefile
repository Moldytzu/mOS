KERNEL := ../out/kernel.elf

# use cross c compiler
CC := ~/cross_compiler/bin/x86_64-elf-gcc

# use cross linker
LD := ~/cross_compiler/bin/x86_64-elf-ld

# use nasm
ASM := nasm

# User controllable CFLAGS.
CFLAGS ?= -O2 -g

# User controllable LDFLAGS
LDFLAGS ?=

# User controllable ASMFLAGS
ASMFLAGS ?= -gdwarf

# Internal C flags that should not be changed by the user.
INTERNALCFLAGS :=   		 \
	-I./src/                 \
	-std=gnu11           	 \
	-ffreestanding       	 \
	-fno-stack-protector 	 \
	-fno-pic             	 \
	-mabi=sysv           	 \
	-mno-red-zone        	 \
	-mcmodel=kernel      	 \
	-Werror 			     \
	-Wno-int-to-pointer-cast \
	-march=nocona            \
	-MMD

# Internal linker flags that should not be changed by the user.
INTERNALLDFLAGS :=         \
	-Tlinker.ld         \
	-nostdlib              \
	-zmax-page-size=0x1000 \
	-static

# Internal ASM flags that should not be changed by the user.
INTERNALASMFLAGS := \
	-felf64

# Use find to glob all *.c files in the directory and extract the object names.
SRCDIR := src
OBJDIR := obj

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

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