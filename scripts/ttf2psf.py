#!/usr/bin/python3
# utility to convert a ttf font to a psf2 font used by the kernel

import sys
import platform
import subprocess


def main():
    if len(sys.argv) != 3:  # not enough arguments
        print(f"Usage: {sys.argv[0]} <input> <output>")
        raise SystemError

    if platform.system() != "Linux" and platform.system() != "Darwin":  # we want only *nix!
        print(
            f"{sys.argv[0]} is designed to only work on *nix-like operating systems!")
        raise SystemError

    # probe for required executables
    try:
        subprocess.call(["otf2bdf", "-h"],
                        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        subprocess.call(["bdf2psf", "-h"],
                        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    except:
        print("Failed to detect otf2bdf and/or bdf2psf!")

    # do the conversion to bdf
    subprocess.call(["otf2bdf", "-p", "16", "-r", "96",
                    "-o", "/tmp/tmp.bdf", sys.argv[1]])

    # grab average width and round it up to next 10 (prevents error because of its uneven nature)
    bdf = open("/tmp/tmp.bdf", "r")
    buf = bdf.read()
    bdf.close()

    numStart = buf.find("AVERAGE_WIDTH ") + len("AVERAGE_WIDTH ")
    
    num = ""
    i = numStart
    while buf[i] != '\n':
        num += buf[i]
        i += 1

    originalLen = len(num)

    num = (int(num) - int(num) % 10)+10 # round it up

    buf = f"{buf[:numStart]}{num}{buf[numStart+originalLen:]}" # patch the font

    # write modification
    bdf = open("/tmp/tmp.bdf", "w")
    bdf.write(buf)
    bdf.close()

    # do the conversion to psf
    subprocess.call(["bdf2psf", "--fb", "/tmp/tmp.bdf",
                    "/usr/share/bdf2psf/standard.equivalents", "/usr/share/bdf2psf/ascii.set", "256", sys.argv[2]])


if __name__ == "__main__":
    main()
