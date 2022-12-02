KERNEL := ../../out/kernel.elf

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
ASMFLAGS ?=

# Internal C flags that should not be changed by the user.
INTERNALCFLAGS :=   		 \
	-I.                  	 \
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
	-T../linker.ld         \
	-nostdlib              \
	-zmax-page-size=0x1000 \
	-static

# Internal ASM flags that should not be changed by the user.
INTERNALASMFLAGS := \
	-felf64

# Use find to glob all *.c files in the directory and extract the object names.
CFILES := $(shell find . -type f -name '*.c')
ASMFILES := $(shell find . -type f -name '*.asm')
OBJ := $(CFILES:.c=.o)
OBJ += $(ASMFILES:.asm=.o)
HEADER_DEPS := $(CFILES:.c=.d)
HEADER_DEPS += $(ASMFILES:.asm=.d)

# Default target.
.PHONY: all
all: $(KERNEL)

limine.h:
	curl https://raw.githubusercontent.com/limine-bootloader/limine/trunk/limine.h -o $@

# Link rules for the final kernel executable.
$(KERNEL): $(OBJ)
	cd obj && $(LD) $(OBJ) $(LDFLAGS) $(INTERNALLDFLAGS) -o $@

# Compilation rules for *.c files.
-include $(HEADER_DEPS)
%.o: %.c
	@ mkdir -p $(dir obj/$@)
	$(CC) $(CFLAGS) $(INTERNALCFLAGS) -c $< -o obj/$@

# Compilation rules for *.asm files.
-include $(HEADER_DEPS)
%.o: %.asm
	@ mkdir -p $(dir obj/$@)
	$(ASM) $(ASMFLAGS) $(INTERNALASMFLAGS) $< -o obj/$@

# Remove object files and the final executable.
.PHONY: clean
clean:
	rm -rf $(KERNEL) $(OBJ) $(HEADER_DEPS) limine.h

.PHONY: setup
setup: limine.h