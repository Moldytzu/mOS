LIB = ../libc.a

# use cross c compiler
CC := ~/cross_compiler/bin/x86_64-elf-gcc

# use cross ar
AR := ~/cross_compiler/bin/x86_64-elf-ar

# use nasm
ASM := nasm

# Internal C flags that should not be changed by the user.
INTERNALCFLAGS :=   		 \
	-I.                  	 \
	-Iinc/                   \
	-std=gnu11           	 \
	-ffreestanding       	 \
	-fno-stack-protector 	 \
	-fno-pic             	 \
	-mabi=sysv           	 \
	-mno-red-zone        	 \
	-Werror 			     \
	-mcmodel=large           \
	-O2

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
all: $(LIB)

# Link rules for the final libc
$(LIB): $(OBJ)
	cd obj && $(AR) -crs $@ $(OBJ)

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