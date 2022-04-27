# use cross c compiler
CC := ~/cross_compiler/bin/x86_64-elf-gcc

# use cross linker
LD := ~/cross_compiler/bin/x86_64-elf-ld

# Internal C flags that should not be changed by the user.
INTERNALCFLAGS :=   		 \
	-I.                  	 \
	-std=gnu11           	 \
	-ffreestanding       	 \
	-fno-stack-protector 	 \
	-fno-pic             	 \
	-mabi=sysv           	 \
	-mno-red-zone        	 \
	-Werror 			     \
	-O2

# Internal linker flags that should not be changed by the user.
INTERNALLDFLAGS :=         \
	-T../../app.ld         \
	-nostdlib              \
	-zmax-page-size=0x1000 \
	-static

# default output path of the app
OUTFOLDER := ../../../roots/initrd/