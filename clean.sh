#!/bin/bash

set -e

find ./drivers/ -type f -name Makefile -execdir make clean \;
find ./kernel/ -type f -name Makefile -execdir make clean \;
find ./apps/ -type f -name Makefile -execdir make clean \;
find ./libs/ -type f -name Makefile -execdir make clean \;
find . -iname "bin" -o -iname "obj" | xargs rm -rf
rm -rf iso_root out roots/img/initrd.dsfs roots/initrd/*.mx roots/initrd/*.drv