OUTPUT = out/dvd.iso
OUTPUTEFI = out/efi.iso
CORES = $(shell nproc)
GDBFLAGS ?= -tui -q -ex "target remote localhost:1234" -ex "layout asm" -ex "tui reg all" -ex "b _start" -ex "continue"
QEMUFLAGS ?= -M q35,smm=off -m 2G -cpu Icelake-Server -serial mon:stdio -D out/qemu.out -d guest_errors,cpu_reset,int -vga vmware
QEMUDEBUG = -no-reboot -no-shutdown -s -S &
APPS = $(wildcard ./apps/*/.)
DRIVERS = $(wildcard ./drivers/*/.)

.PHONY: all run run-debug run-efi run-efi-debug limine ovmf kernel efi clean deps initrd libc

all: $(OUTPUT)

run: $(OUTPUT)
	qemu-system-x86_64 $(QEMUFLAGS) -boot d -cdrom $(OUTPUT)

run-kvm: $(OUTPUT)
	qemu-system-x86_64 $(QEMUFLAGS) -boot d -cdrom $(OUTPUT) --enable-kvm -cpu host

run-debug: $(OUTPUT)
	qemu-system-x86_64 $(QEMUFLAGS) -boot d -cdrom $(OUTPUT) $(QEMUDEBUG)
	gdb-multiarch $(GDBFLAGS) out/kernel.elf
	pkill -f qemu-system-x86_64

run-efi: efi
	qemu-system-x86_64 -bios ovmf/ovmf.fd $(QEMUFLAGS) -cdrom $(OUTPUTEFI)

run-efi-kvm: efi
	qemu-system-x86_64 -bios ovmf/ovmf.fd $(QEMUFLAGS) -cdrom $(OUTPUTEFI) --enable-kvm -cpu host

run-efi-debug: efi
	qemu-system-x86_64 -bios ovmf/ovmf.fd $(QEMUFLAGS) -cdrom $(OUTPUTEFI) -no-reboot -no-shutdown -d int -M smm=off -D out/qemu.out -s -S &
	gdb-multiarch $(GDBFLAGS) out/kernel.elf
	pkill -f qemu-system-x86_64

limine:
	-git clone https://github.com/limine-bootloader/limine.git --branch=v4.x-branch-binary --depth=1
	make -C limine

initrd:
	python3 ./scripts/dsfs.py roots/initrd/ roots/img/initrd.dsfs

FORCE:

# run make in every folder from apps and drivers
$(APPS): FORCE
	$(MAKE) -C $@ 

$(DRIVERS): FORCE
	$(MAKE) -C $@ 

# make the libc
libc:
	$(MAKE) -C libc

kernel:
	mkdir -p out
	$(MAKE) -C kernel setup
	$(MAKE) -C kernel -j$(CORES)

$(OUTPUT): limine kernel libc $(APPS) $(DRIVERS) initrd 
	rm -rf iso_root
	mkdir -p iso_root
	cp out/kernel.elf limine/limine.sys limine/limine-cd.bin limine/limine-cd-efi.bin iso_root/
	cd roots/img && cp -r . ../../iso_root
	xorriso -as mkisofs -b limine-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-cd-efi.bin \
		-efi-boot-part --efi-boot-image -J \
		iso_root -o $(OUTPUT)
	limine/limine-deploy $(OUTPUT)
	rm -rf iso_root

efi: limine kernel initrd $(APPS) $(DRIVERS)
	rm -rf iso_root
	mkdir -p iso_root
	cp out/kernel.elf limine/limine.sys limine/limine-cd-efi.bin iso_root/
	cd roots/img && cp -r . ../../iso_root
	xorriso -as mkisofs --efi-boot limine-cd-efi.bin -efi-boot-part --efi-boot-image -J -o $(OUTPUTEFI) iso_root
	rm -rf iso_root

clean:
	rm -rf iso_root $(OUTPUT) cross-compiler-builder limine out roots/img/initrd.dsfs roots/initrd/*.mx
	$(MAKE) -C kernel clean

apt:
	sudo apt update
	sudo apt upgrade -y

deps: apt
	sudo apt install gdb gdb-multiarch build-essential nasm xorriso qemu-system-x86 python3 -y

toolchain:
	-git clone https://github.com/Moldytzu/cross-compiler-builder.git
	/bin/bash cross-compiler-builder/buildcrosscompiler.sh
