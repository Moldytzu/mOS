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
	gdb -tui -q -ex "target remote localhost:1234" -ex "layout asm" -ex "b _start" -ex "continue" out/kernel.elf
	pkill -f qemu-system-x86_64

.PHONY: run-efi
run-efi: efi ovmf
	qemu-system-x86_64 -bios ovmf/ovmf.fd -M q35 -m 2G -cdrom $(OUTPUTEFI)

.PHONY: run-efi
run-efi-debug: efi ovmf
	qemu-system-x86_64 -bios ovmf/ovmf.fd -M q35 -m 2G -cdrom $(OUTPUTEFI) -no-reboot -no-shutdown -d int -M smm=off -D out/qemu.out -s -S &
	gdb -tui -q -ex "target remote localhost:1234" -ex "layout asm" -ex "b _start" -ex "continue" out/kernel.elf
	pkill -f qemu-system-x86_64

.PHONY: limine
limine:
	-git clone https://github.com/limine-bootloader/limine.git --branch=v3.0-branch-binary --depth=1
	make -C limine

.PHONY: ovmf
ovmf:
	mkdir -p ovmf
	-wget -nc https://retrage.github.io/edk2-nightly/bin/RELEASEX64_OVMF.fd -O ovmf/ovmf.fd

.PHONY: kernel
kernel:
	mkdir -p out
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
	limine/limine-deploy $(OUTPUT)
	rm -rf iso_root

.PHONY: efi
efi: limine kernel
	rm -rf iso_root
	mkdir -p iso_root
	cp out/kernel.elf limine.cfg font-8x16.psf limine/limine-cd-efi.bin iso_root/
	xorriso -as mkisofs --efi-boot limine-cd-efi.bin -efi-boot-part --efi-boot-image -J -o $(OUTPUTEFI) iso_root
	rm -rf iso_root

.PHONY: clean
clean:
	rm -rf iso_root $(OUTPUT) limine ovmf
	$(MAKE) -C kernel clean