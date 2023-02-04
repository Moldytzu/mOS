# Building the whole system

## Prerequisites
First things first clone this repository using git. This shouldn't take long but if it does make sure to pass `--depth 1` to it.

The bare minimum hardware to build and emulate mOS is a Debian machine (could work on other distros) with enough disk space to build the toolchain (minimum 2 GB) and a reliable internet connection. Altough the architecture doesn't matter (yes, you can compile on a Pi 4), an x86_64 machine is recommended.

If you are on Debian, there is a script that installs all the required packages automatically. To run it do `make deps`. (sudo permissions are required!)

## The toolchain
Make sure you're in the root of the source tree (the folder containing the license agreement)!

Run `make toolchain` and expect to wait at least half a hour. When done you will have it installed in your home folder at ~/cross_compiler/. Don't worry, you don't have to add anything to path because the build system will automatically pick the binaries from that folder.

## Actually building the system
You simply have to do `make`. That's all. The kernel will be compiled to out/kernel.elf and an iso to out/dvd.iso.

## Testing in QEmu
There are multiple targets that can used to test the operating system in QEmu:
    - run  -> runs QEmu with TCG
    - run-efi  -> runs QEmu with TCG and UEFI support
    - run-kvm  -> runs QEmu with KVM support (only on x86_64 hosts)
    - run-efi-kvm  -> runs QEmu with KVM and UEFI support (only on x86_64 hosts)
    - run-debug  -> runs QEMU with TCG and gdb attached
    - run-efi-debug  -> runs QEMU with UEFI support and gdb attached