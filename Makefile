CORES = $(shell nproc)
DISK = image.disk
GDBFLAGS ?= -tui -q -x gdb.script
QEMUFLAGS ?= -M q35,smm=off -m 512M -smp 4 -cpu core2duo -hda $(DISK) -boot c -serial mon:stdio -D out/qemu.out -d guest_errors,cpu_reset,int,trace:ahci_dma_prepare_buf_fail,trace:ahci_cmd_done,trace:ahci_start_dma,trace:ahci_dma_prepare_buf,trace:ahci_dma_rw_buf -vga vmware
QEMUDEBUG = -smp 1 -no-reboot -no-shutdown -s -S
APPS = $(wildcard ./apps/*/.)
DRIVERS = $(wildcard ./drivers/*/.)

.PHONY: all run run-debug run-efi run-efi-debug limine ovmf kernel efi clean deps initrd libc

all: image

update-ovmf:
	wget https://retrage.github.io/edk2-nightly/bin/RELEASEX64_OVMF.fd -O ovmf/ovmf.fd

run: image
	qemu-system-x86_64 $(QEMUFLAGS) 

run-smp-debug: image
	qemu-system-x86_64 $(QEMUFLAGS) $(QEMUDEBUG) -smp 4 &
	gdb-multiarch -tui -q -x smpgdb.script out/kernel.elf
	pkill -f qemu-system-x86_64
	reset

run-no-smp: image
	qemu-system-x86_64 -M q35,smm=off -cpu core2duo -hda $(DISK) -boot c -serial mon:stdio -D out/qemu.out -d guest_errors,cpu_reset,int -vga vmware -m 512M

run-bochs: image
	bochs -q

run-kvm: image
	qemu-system-x86_64 $(QEMUFLAGS) --enable-kvm -cpu host -smp $(CORES)

run-debug: image
	qemu-system-x86_64 $(QEMUFLAGS) $(QEMUDEBUG) &
	gdb-multiarch $(GDBFLAGS) out/kernel.elf
	pkill -f qemu-system-x86_64
	reset

run-efi:
	qemu-system-x86_64 -bios ovmf/ovmf.fd $(QEMUFLAGS) 

run-efi-kvm:
	qemu-system-x86_64 -bios ovmf/ovmf.fd $(QEMUFLAGS) --enable-kvm -cpu host

run-efi-debug:
	qemu-system-x86_64 -bios ovmf/ovmf.fd $(QEMUFLAGS) -no-reboot -no-shutdown -d int -M smm=off -D out/qemu.out -s -S &
	gdb-multiarch $(GDBFLAGS) out/kernel.elf
	pkill -f qemu-system-x86_64
	reset

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

# create mbr partition table and format a primary fat32 partition
$(DISK):
	dd if=/dev/zero of=./$(DISK) bs=1M count=128
	parted -s ./$(DISK) mklabel msdos
	parted -s ./$(DISK) mkpart primary fat32 1% 100%
	sudo kpartx -a ./$(DISK)
	sudo mkfs -t vfat /dev/mapper/loop*p1
	sudo kpartx -d ./$(DISK)

image: $(DISK) limine kernel libc $(APPS) $(DRIVERS) initrd 
	sudo kpartx -a ./$(DISK)
	sudo mkdir -p /mnt
	sudo mkdir -p /mnt/mOS
	sudo mkdir -p /mnt/mOS/EFI
	sudo mkdir -p /mnt/mOS/EFI/BOOT
	sudo mount /dev/mapper/loop*p1 /mnt/mOS
	sudo cp out/kernel.elf limine/limine.sys /mnt/mOS/
	sudo cp limine/BOOTX64.EFI /mnt/mOS/EFI/BOOT/
	cd roots/img && sudo cp -r . /mnt/mOS
	limine/limine-deploy ./$(DISK)
	sudo umount /dev/mapper/loop*p1
	sudo kpartx -d ./$(DISK)

mount: 
	sudo kpartx -a ./$(DISK)
	sudo mkdir -p /mnt
	sudo mkdir -p /mnt/mOS
	sudo mount /dev/mapper/loop*p1 /mnt/mOS

umount:
	sudo umount /dev/mapper/loop*p1
	sudo kpartx -d ./$(DISK)

clean:
	chmod +x ./clean.sh
	bash ./clean.sh

apt:
	sudo apt update
	sudo apt upgrade -y

deps: apt
	sudo apt install gdb gdb-multiarch build-essential nasm xorriso qemu-system-x86 python3 -y

toolchain:
	-git clone https://github.com/Moldytzu/cross-compiler-builder.git
	/bin/bash cross-compiler-builder/buildcrosscompiler.sh
