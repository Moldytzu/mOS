OUTPUT = out/dvd.iso
OUTPUTEFI = out/efi.iso
CORES = $(shell nproc)

.PHONY: all
all: $(OUTPUT)

.PHONY: run
run: $(OUTPUT)
	qemu-system-x86_64 -M q35 -m 2G -cdrom $(OUTPUT) -boot d

.PHONY: run-debug
run-debug: $(OUTPUT)
	qemu-system-x86_64 -M q35 -m 2G -cdrom $(OUTPUT) -boot d -no-reboot -no-shutdown -d int -M smm=off -D out/qemu.out -s -S &
	gdb -q -ex "target remote localhost:1234" -ex "b _start" -ex "continue" out/kernel.elf

limine:
	mkdir -p out
	git clone https://github.com/limine-bootloader/limine.git --branch=latest-binary --depth=1
	make -C limine

.PHONY: kernel
kernel:
	$(MAKE) -C kernel -j$(CORES)

$(OUTPUT): limine kernel
	rm -rf iso_root
	mkdir -p iso_root
	cp out/kernel.elf \
		limine.cfg limine/limine.sys limine/limine-cd.bin limine/limine-cd-efi.bin font-8x16.psf iso_root/
	xorriso -as mkisofs -b limine-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-cd-efi.bin \
		-efi-boot-part --efi-boot-image -J \
		iso_root -o $(OUTPUT)
	limine/limine-s2deploy $(OUTPUT)
	rm -rf iso_root

efi: limine kernel
	rm -rf iso_root
	mkdir -p iso_root
	mkdir -p iso_root/EFI
	mkdir -p iso_root/EFI/BOOT
	cp out/kernel.elf limine.cfg font-8x16.psf iso_root/
	cp limine/BOOTX64.EFI iso_root/EFI/BOOT/
	xorrisofs -r -J -o $(OUTPUTEFI) iso_root
	limine/limine-s2deploy $(OUTPUT)
	rm -rf iso_root

.PHONY: clean
clean:
	rm -rf iso_root $(OUTPUT) limine
	$(MAKE) -C kernel clean