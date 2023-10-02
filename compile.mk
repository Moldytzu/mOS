# prefix
PREFIX := ~/cross_compiler/bin/x86_64-elf-

# c compiler
CC := $(PREFIX)gcc

# archiver
AR := $(PREFIX)ar

# linker
LD := $(PREFIX)ld

# assembler
ASM := nasm

# flags for c compiler
CFLAGS :=   		              \
	-I.                  	      \
	-Iinc/                        \
	-std=gnu11           	      \
	-ffreestanding       	      \
	-fno-stack-protector 	      \
	-fno-pic             	      \
	-mabi=sysv           	      \
	-mno-red-zone        	      \
	-Werror 			          \
	-Wno-address-of-packed-member \
	-mcmodel=kernel               \
	-Og                           \
	-march=nocona                 \
	-MMD                          \
	-g

# flags for assembler
ASMFLAGS :=    \
	-felf64    \
	-gdwarf    \
	-isrc/cpu/

# flags for linker
LDFLAGS := 				   \
	-zmax-page-size=0x1000 \

# Use find to glob all *.c files in the directory and extract the object names.
SRCDIR := src
OBJDIR := obj

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))