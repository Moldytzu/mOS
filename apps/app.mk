# use cross c compiler
CC := ~/cross_compiler/bin/x86_64-elf-gcc

# use cross linker
LD := ~/cross_compiler/bin/x86_64-elf-ld

# Internal C flags that should not be changed by the user.
INTERNALCFLAGS :=   		 \
	-I.                  	 \
	-I../../libc/inc         \
	-std=gnu11           	 \
	-ffreestanding       	 \
	-fno-stack-protector 	 \
	-fno-pic             	 \
	-mabi=sysv           	 \
	-mno-red-zone        	 \
	-Werror 			     \
	-mcmodel=large           \
	-msse                    \
	-msse2                   \
	-msse3                   \
	-msse4                   \
	-msse4.1                 \
	-msse4.2                 \
	-O2

# Internal linker flags that should not be changed by the user.
INTERNALLDFLAGS :=         \
	-T../../app.ld         \
	-nostdlib              \
	-zmax-page-size=0x1000 \
	-static                \
	../../../libc/libc.a

# default output path of the app
OUTFOLDER := ../../../roots/initrd/