#!/bin/sh

git clone https://github.com/Moldytzu/mdoomgeneric
cd mdoomgeneric
make -C doomgeneric -f Makefile.mos
cd ..
cp ./mdoomgeneric/doomgeneric/doomgeneric ../../roots/initrd/doom.mx