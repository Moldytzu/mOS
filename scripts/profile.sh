#!/bin/bash
# modified version of https://gist.github.com/mintsuki/2079737d76f30c7dae1e8cb791325082#file-qemu-prof-sh

set -e

ARCH="x86_64"
INTERVAL="0.001"
CMDLINE="-M q35 -m 2G -boot d -cdrom ../out/dvd.iso"
KERNEL="../out/kernel.elf"

SOCKET="$(mktemp)"
OUTFILE="$(mktemp)"
SUBSHELL="$(mktemp)"

trap "rm -f '$SOCKET' '$OUTFILE' '$SUBSHELL'" EXIT INT TERM HUP

(
echo $BASHPID >"$SUBSHELL"
while true; do
    echo "info registers" >>"$SOCKET"
    sleep $INTERVAL
done
) &

tail -f "$SOCKET" | qemu-system-"$ARCH" $CMDLINE -monitor stdio \
    | grep -o '[RE]IP=[0-9a-f]*' \
    | sed 's/[RE]IP=//g' \
    | sort | uniq -c | sort -nr >"$OUTFILE"

kill $(cat "$SUBSHELL")

ISLINENUM=1
for p in $(cat "$OUTFILE"); do
    if [ $ISLINENUM = 1 ]; then
        ISLINENUM=0
        echo -ne "$p\t"
        continue
    fi

    if ! [ -z "$KERNEL" ]; then
        ~/cross_compiler/bin/x86_64-elf-addr2line -e "$KERNEL" "0x$p" | tr -d '\n'
        echo " (0x$p)"
    else
        echo "(0x$p)"
    fi

    ISLINENUM=1
done