include ../../compile.mk

# Internal C flags that should not be changed by the user.
INTERNALCFLAGS :=   		 \
	-I../../libc/inc         \
	-mcmodel=large       

# Internal linker flags that should not be changed by the user.
INTERNALLDFLAGS :=         \
	-T../app.ld            \
	-nostdlib              \
	../../libc.a

# default output path of the app
OUTFOLDER := ../../roots/initrd/