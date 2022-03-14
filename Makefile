OUTPUT = out/dvd.iso

.PHONY: all
all: $(OUTPUT)

.PHONY: run
run: $(OUTPUT)
	qemu-system-x86_64 -M q35 -m 2G -cdrom $(OUTPUT) -boot d

.PHONY: run-debug
run-debug: $(OUTPUT)
	qemu-system-x86_64 -M q35 -m 2G -cdrom $(OUTPUT) -boot d -no-reboot -no-shutdown -d int

limine:
	mkdir -p out
	git clone https://github.com/limine-bootloader/limine.git --branch=v2.0-branch-binary --depth=1
	make -C limine

.PHONY: kernel
kernel:
	$(MAKE) -C kernel

$(OUTPUT): limine kernel
	rm -rf iso_root
	mkdir -p iso_root
	cp out/kernel.elf \
		limine.cfg limine/limine.sys limine/limine-cd.bin limine/limine-eltorito-efi.bin font-8x16.psf iso_root/
	xorriso -as mkisofs -b limine-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-eltorito-efi.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o $(OUTPUT)
	limine/limine-install $(OUTPUT)
	rm -rf iso_root

.PHONY: clean
clean:
	rm -f iso_root $(OUTPUT)
	$(MAKE) -C kernel clean

.PHONY: distclean
distclean: clean
	rm -rf limine
	$(MAKE) -C kernel distclean
