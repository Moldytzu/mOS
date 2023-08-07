include ../../compile.mk

# here goes all the boilerplate/helpers

SRCC = $(call rwildcard,$(SRCDIR),*.c) # c source files
ASMSRC = $(call rwildcard,$(SRCDIR),*.asm) # assembly source files
OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCC)) # generate object names for c
OBJS += $(patsubst $(SRCDIR)/%.asm, $(OBJDIR)/%_asm.o, $(ASMSRC)) # do the same for assembly
DIRS = $(wildcard $(SRCDIR)/*) # inner source folders

APPNAME = $(notdir $(shell pwd))

# default output path of the app
OUTFOLDER := ../../roots/initrd/
OUTFILE := $(OUTFOLDER)$(APPNAME).drv # generate output name

# Internal C flags that should not be changed by the user.
INTERNALCFLAGS :=   		 \
	-mcmodel=large           \
	-I./src/

# Internal linker flags that should not be changed by the user.
INTERNALLDFLAGS :=         \
	-T../driver.ld         \
	-nostdlib              \
	-static                

define parse_libs
$(foreach lib, $(1), \
	$(eval LDFLAGS+= ../../libs/$(lib)/bin/$(lib).a) \
	$(eval CFLAGS+=-I../../libs/$(lib)/inc ) \
)
endef

# default targets
.PHONY: app

# link the app
app: $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) $(INTERNALLDFLAGS) -o $(OUTFILE)

# compile .c files
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@ mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INTERNALCFLAGS) -c $^ -o $@

# assemble .asm files
$(OBJDIR)/%_asm.o: $(SRCDIR)/%.asm
	@ mkdir -p $(dir $@)
	$(ASM) $(ASMFLAGS) $(INTERNALASMFLAGS) $^ -o $@
	
clean:
	rm -rf $(OBJS) $(OUTFILE)