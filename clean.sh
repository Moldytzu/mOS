set -e

find ./libc/ -type f -name Makefile -execdir make clean \;
find ./drivers/ -type f -name Makefile -execdir make clean \;
find ./kernel/ -type f -name Makefile -execdir make clean \;
find ./apps/ -type f -name Makefile -execdir make clean \;
find . -iname "bin" -o -iname "obj" | xargs rm -rf
rm -rf iso_root out cross-compiler-builder limine out roots/img/initrd.dsfs roots/initrd/*.mx roots/initrd/*.drv