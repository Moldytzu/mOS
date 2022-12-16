include ../app.mk

# append the output filename
OUTFILE = $(OUTFOLDER)vmsvga.drv

# the C sources
CFILES := $(shell find . -type f -name '*.c')

# the object files
OBJ := $(CFILES:.c=.o)

# the header deps
HEADER_DEPS := $(CFILES:.c=.d)

.PHONY: app

# link the app
app: $(OBJ)
	cd obj && $(LD) $(OBJ) $(INTERNALLDFLAGS) -o $(OUTFILE)

# the actual compilation rule
-include $(HEADER_DEPS)
%.o: %.c
	@ mkdir -p $(dir obj/$@)
	$(CC) $(INTERNALCFLAGS) -c $< -o obj/$@