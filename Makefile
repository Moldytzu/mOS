OUTPUT = out/dvd.iso
OUTPUTEFI = out/efi.iso
CORES = $(shell nproc)

.PHONY: all run run-debug run-efi run-efi-debug limine ovmf kernel efi clean deps

all: $(OUTPUT)

run: $(OUTPUT)
	qemu-system-x86_64 -M q35 -m 2G -cdrom $(OUTPUT) -boot d

run-debug: $(OUTPUT)
	qemu-system-x86_64 -M q35 -m 2G -cdrom $(OUTPUT) -boot d -no-reboot -no-shutdown -d int -M smm=off -D out/qemu.out -s -S &
	gdb -tui -q -ex "target remote localhost:1234" -ex "layout asm" -ex "tui reg all" -ex "b _start" -ex "continue" out/kernel.elf
	pkill -f qemu-system-x86_64

run-efi: efi ovmf
	qemu-system-x86_64 -bios ovmf/ovmf.fd -M q35 -m 2G -cdrom $(OUTPUTEFI)

run-efi-debug: efi ovmf
	qemu-system-x86_64 -bios ovmf/ovmf.fd -M q35 -m 2G -cdrom $(OUTPUTEFI) -no-reboot -no-shutdown -d int -M smm=off -D out/qemu.out -s -S &
	gdb -tui -q -ex "target remote localhost:1234" -ex "layout asm" -ex "tui reg all" -ex "b _start" -ex "continue" out/kernel.elf
	pkill -f qemu-system-x86_64

limine:
	-git clone https://github.com/limine-bootloader/limine.git --branch=v3.0-branch-binary --depth=1
	make -C limine

ovmf:
	mkdir -p ovmf
	-wget -nc https://retrage.github.io/edk2-nightly/bin/RELEASEX64_OVMF.fd -O ovmf/ovmf.fd

kernel:
	mkdir -p out kernel/obj
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

efi: limine kernel
	rm -rf iso_root
	mkdir -p iso_root
	cp out/kernel.elf limine.cfg font-8x16.psf limine/limine-cd-efi.bin iso_root/
	xorriso -as mkisofs --efi-boot limine-cd-efi.bin -efi-boot-part --efi-boot-image -J -o $(OUTPUTEFI) iso_root
	rm -rf iso_root

clean:
	rm -rf iso_root $(OUTPUT) limine ovmf
	$(MAKE) -C kernel clean

deps:
	sudo apt install build-essential nasm xorriso qemu-system-x86 -y
