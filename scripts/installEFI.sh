PARTITION=$1
SRC=$(pwd)

mkdir -p $PARTITION/EFI
mkdir -p $PARTITION/EFI/BOOT
cp $SRC/limine/BOOTX64.EFI $PARTITION/EFI/BOOT
cp $SRC/roots/img/* $PARTITION/
cp $SRC/out/kernel.elf $PARTITION/

echo Installed for EFI on $PARTITION