include ../app.mk

# append the output filename
OUTFILE = $(OUTFOLDER)crash.mx

SRCC = $(call rwildcard,$(SRCDIR),*.c)  
ASMSRC = $(call rwildcard,$(SRCDIR),*.asm)  
OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCC))
OBJS += $(patsubst $(SRCDIR)/%.asm, $(OBJDIR)/%_asm.o, $(ASMSRC))
DIRS = $(wildcard $(SRCDIR)/*)

.PHONY: app

# link the app
app: $(OBJS)
	$(LD) $(OBJS) $(INTERNALLDFLAGS) -o $(OUTFILE)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@ mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INTERNALCFLAGS) -c $^ -o $@

# Compilation rules for *.asm files.
$(OBJDIR)/%_asm.o: $(SRCDIR)/%.asm
	@ mkdir -p $(dir $@)
	$(ASM) $(ASMFLAGS) $(INTERNALASMFLAGS) $^ -o $@
	
clean:
	rm -rf $(OUTFILE)