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
	-O2                      \
	-march=nocona            

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
all: $(LIB)

# Link rules for the final libc
$(LIB): $(OBJS)
	$(AR) -crs $@ $(OBJS)

# Compilation rules for *.c files.
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@ mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INTERNALCFLAGS) -c $^ -o $@

# Compilation rules for *.asm files.
$(OBJDIR)/%_asm.o: $(SRCDIR)/%.asm
	@ mkdir -p $(dir $@)
	$(ASM) $(ASMFLAGS) $(INTERNALASMFLAGS) $^ -o $@

clean:
	rm -rf $(LIB)