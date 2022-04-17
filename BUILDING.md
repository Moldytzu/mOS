# Building & running on Debian/Ubuntu
    - make sure you have all the dependencies installed by running `make deps`, this will also upgrade all the packages you have installed
    - build the toolchain by running `make toolchain` (this will take a while)
    - run `make run` to build the operating system disk and binaries and to run qemu.
    - alternatively you can run qemu with UEFI support by making the run-efi target, but the standard BIOS method has a faster startup

# Debugging using gdb
    - run `make run-debug` to hook gdb to qemu.
